from flask import Flask, render_template, request, jsonify
import secrets
import json
import subprocess
import os
import sys
import logging
from datetime import datetime
from logging.handlers import RotatingFileHandler

app = Flask(__name__)
app.secret_key = secrets.token_hex(16)

games = {}

LOG_DIR = os.path.join(os.path.dirname(__file__), 'logs')
if not os.path.exists(LOG_DIR):
    os.makedirs(LOG_DIR)

LOG_FILE = os.path.join(LOG_DIR, 'app.log')

logging.basicConfig(
    level=logging.DEBUG,
    format='%(asctime)s [%(levelname)s] %(message)s',
    handlers=[
        RotatingFileHandler(LOG_FILE, maxBytes=10*1024*1024, backupCount=5),
        logging.StreamHandler()
    ]
)

logger = logging.getLogger(__name__)
logger.info("=" * 60)
logger.info("Application started")
logger.info(f"Log file: {LOG_FILE}")
logger.info("=" * 60)

BASE_DIR = os.path.dirname(__file__)
if os.name == 'nt':
    WEB_CLI_PATH = os.path.join(BASE_DIR, '..', 'build', 'web_cli.exe')
else:
    WEB_CLI_PATH = os.path.join(BASE_DIR, '..', 'build', 'web_cli')


def call_cpp_engine(command, win_length, moves, current_player, time_ms=5000, move_x=None, move_y=None):
    logger.info(f"[C++ CALL] command={command}, win_length={win_length}, current_player={current_player}, "
                f"moves_count={len(moves)}, time_ms={time_ms}, move=({move_x}, {move_y})")
    
    try:
        input_data = {
            'command': command,
            'win_length': win_length,
            'moves': moves,
            'current_player': current_player,
            'time_ms': time_ms
        }
        
        if move_x is not None and move_y is not None:
            input_data['x'] = move_x
            input_data['y'] = move_y
        
        input_json = json.dumps(input_data)
        logger.debug(f"[C++ INPUT] {input_json[:500]}")
        
        process = subprocess.Popen(
            [WEB_CLI_PATH],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        
        stdout, stderr = process.communicate(input=input_json, timeout=30)
        
        logger.debug(f"[C++ STDOUT] {stdout[:500] if stdout else '(empty)'}")
        if stderr:
            logger.warning(f"[C++ STDERR] {stderr[:500]}")
        
        if process.returncode != 0:
            error_msg = stderr.strip() if stderr else "Unknown error"
            try:
                error_json = json.loads(stdout)
                if 'error' in error_json:
                    error_msg = error_json['error']
            except:
                if stdout.strip():
                    error_msg += f" | stdout: {stdout.strip()[:200]}"
            logger.error(f"[C++ ERROR] returncode={process.returncode}, error={error_msg}")
            return {'success': False, 'error': f'Engine failed: {error_msg}'}
        
        try:
            result = json.loads(stdout)
            logger.info(f"[C++ SUCCESS] command={command}, result_keys={list(result.keys())}")
            if 'stats' in result:
                stats = result['stats']
                logger.debug(f"[C++ STATS] time_ms={stats.get('time_ms')}, "
                           f"decision_type={stats.get('decision_type')}, "
                           f"depth={stats.get('depth_reached')}")
            return result
        except json.JSONDecodeError as e:
            logger.error(f"[C++ PARSE ERROR] {str(e)}, stdout={stdout[:200]}")
            return {'success': False, 'error': f'Failed to parse engine output: {str(e)}. Output: {stdout[:200]}'}
        
    except subprocess.TimeoutExpired:
        logger.error(f"[C++ TIMEOUT] command={command}")
        return {'success': False, 'error': 'Engine timeout (30 seconds)'}
    except FileNotFoundError:
        logger.error(f"[C++ NOT FOUND] path={WEB_CLI_PATH}")
        return {'success': False, 'error': f'Engine not found at {WEB_CLI_PATH}. Please build the C++ project first.'}
    except Exception as e:
        logger.exception(f"[C++ EXCEPTION] command={command}, error={str(e)}")
        return {'success': False, 'error': f'Unexpected error: {str(e)}'}


@app.route('/')
def index():
    logger.debug("[API] GET /")
    return render_template('index.html')


@app.route('/api/new_game', methods=['POST'])
def new_game():
    logger.info(f"[API] POST /api/new_game")
    
    if not request.json:
        logger.warning("[API] new_game: No JSON data")
        return jsonify({'error': 'JSON data required'}), 400
    
    data = request.json
    logger.debug(f"[API] new_game data: {json.dumps(data)}")
    
    win_length = data.get('win_length', 5)
    human_player = data.get('human_player', 'X')
    ai_time_ms = data.get('ai_time_ms', 5000)
    first_player = data.get('first_player', 'human')
    
    if not (3 <= win_length <= 20):
        return jsonify({'error': 'Win length must be between 3 and 20'}), 400
    
    if human_player not in ['X', 'O']:
        return jsonify({'error': 'Human player must be X or O'}), 400
    
    if not (100 <= ai_time_ms <= 30000):
        return jsonify({'error': 'AI time must be between 100 and 30000 ms'}), 400
    
    game_id = secrets.token_hex(8)
    logger.info(f"[GAME] Created game_id={game_id}, win_length={win_length}, "
                f"human_player={human_player}, first_player={first_player}")
    
    games[game_id] = {
        'win_length': win_length,
        'human_player': human_player,
        'ai_player': 'O' if human_player == 'X' else 'X',
        'ai_time_ms': ai_time_ms,
        'current_player': 'X',
        'moves': [],
        'game_over': False,
        'winner': None
    }
    
    if first_player == 'ai':
        logger.info(f"[GAME] AI makes first move for game_id={game_id}")
        ai_result = call_cpp_engine(
            'ai_move',
            win_length,
            [],
            games[game_id]['ai_player'],
            ai_time_ms
        )
        
        if ai_result.get('success'):
            ai_move_x = ai_result['move']['x']
            ai_move_y = ai_result['move']['y']
            games[game_id]['moves'].append({
                'x': ai_move_x,
                'y': ai_move_y,
                'player': games[game_id]['ai_player']
            })
            games[game_id]['current_player'] = human_player
            games[game_id]['game_over'] = ai_result.get('game_over', False)
            games[game_id]['winner'] = ai_result.get('winner')
            
            logger.info(f"[GAME] AI first move: ({ai_move_x}, {ai_move_y}), "
                       f"new_current={human_player}, game_over={games[game_id]['game_over']}")
            
            return jsonify({
                'game_id': game_id,
                'board': ai_result['board'],
                'current_player': human_player,
                'move': ai_result['move'],
                'stats': ai_result.get('stats'),
                'game_over': games[game_id]['game_over'],
                'winner': games[game_id]['winner']
            })
        else:
            logger.error(f"[GAME] AI first move failed: {ai_result.get('error')}")
    
    state_result = call_cpp_engine('get_state', win_length, [], 'X')
    
    if not state_result.get('success'):
        logger.error(f"[API] new_game: Failed to get initial state: {state_result.get('error')}")
        return jsonify(state_result), 500
    
    logger.info(f"[API] new_game: Successfully created game_id={game_id}")
    
    return jsonify({
        'game_id': game_id,
        'board': state_result.get('board', {'cells': [], 'bbox': {'min_x': 0, 'max_x': 0, 'min_y': 0, 'max_y': 0}}),
        'current_player': 'X',
        'game_over': False,
        'winner': None
    })


@app.route('/api/make_move', methods=['POST'])
def make_move():
    logger.info(f"[API] POST /api/make_move")
    
    if not request.json:
        logger.warning("[API] make_move: No JSON data")
        return jsonify({'error': 'JSON data required'}), 400
    
    data = request.json
    game_id = data.get('game_id')
    x = data.get('x')
    y = data.get('y')
    
    logger.info(f"[API] make_move: game_id={game_id}, move=({x}, {y})")
    
    if game_id not in games:
        logger.warning(f"[API] make_move: Game not found, game_id={game_id}")
        return jsonify({'error': 'Game not found'}), 404
    
    game = games[game_id]
    logger.debug(f"[GAME] State: current_player={game['current_player']}, "
                f"human_player={game['human_player']}, moves_count={len(game['moves'])}, "
                f"game_over={game['game_over']}")
    
    if game['game_over']:
        logger.warning(f"[API] make_move: Game is over, game_id={game_id}")
        return jsonify({'error': 'Game is over'}), 400
    
    if x is None or y is None:
        logger.warning(f"[API] make_move: Missing coordinates, x={x}, y={y}")
        return jsonify({'error': 'x and y coordinates required'}), 400
    
    if game['current_player'] != game['human_player']:
        logger.warning(f"[API] make_move: Not player's turn, current={game['current_player']}, "
                      f"human={game['human_player']}")
        return jsonify({'error': 'Not your turn'}), 400
    
    result = call_cpp_engine(
        'make_move',
        game['win_length'],
        game['moves'],
        game['current_player'],
        game['ai_time_ms'],
        x,
        y
    )
    
    if not result.get('success'):
        logger.error(f"[API] make_move failed: {result.get('error')}")
        return jsonify(result), 500
    
    game['moves'].append({
        'x': x,
        'y': y,
        'player': game['current_player']
    })
    game['current_player'] = game['ai_player']
    game['game_over'] = result.get('game_over', False)
    game['winner'] = result.get('winner')
    
    logger.info(f"[GAME] Move made: ({x}, {y}) by {game['moves'][-1]['player']}, "
                f"new_current={game['current_player']}, game_over={game['game_over']}, "
                f"winner={game['winner']}, total_moves={len(game['moves'])}")
    
    return jsonify({
        'board': result['board'],
        'current_player': game['current_player'],
        'move': result.get('move'),
        'game_over': game['game_over'],
        'winner': game['winner']
    })


@app.route('/api/ai_move', methods=['POST'])
def ai_move():
    logger.info(f"[API] POST /api/ai_move")
    
    if not request.json:
        logger.warning("[API] ai_move: No JSON data")
        return jsonify({'error': 'JSON data required'}), 400
    
    data = request.json
    game_id = data.get('game_id')
    
    logger.info(f"[API] ai_move: game_id={game_id}")
    
    if game_id not in games:
        logger.warning(f"[API] ai_move: Game not found, game_id={game_id}")
        return jsonify({'error': 'Game not found'}), 404
    
    game = games[game_id]
    logger.debug(f"[GAME] State: current_player={game['current_player']}, "
                f"ai_player={game['ai_player']}, moves_count={len(game['moves'])}, "
                f"game_over={game['game_over']}")
    
    if game['game_over']:
        logger.warning(f"[API] ai_move: Game is over, game_id={game_id}")
        return jsonify({'error': 'Game is over'}), 400
    
    if game['current_player'] != game['ai_player']:
        logger.warning(f"[API] ai_move: Not AI turn, current={game['current_player']}, "
                      f"ai={game['ai_player']}")
        return jsonify({'error': 'Not AI turn'}), 400
    
    result = call_cpp_engine(
        'ai_move',
        game['win_length'],
        game['moves'],
        game['current_player'],
        game['ai_time_ms']
    )
    
    if not result.get('success'):
        logger.error(f"[API] ai_move failed: {result.get('error')}")
        return jsonify(result), 500
    
    ai_move_x = result['move']['x']
    ai_move_y = result['move']['y']
    game['moves'].append({
        'x': ai_move_x,
        'y': ai_move_y,
        'player': game['current_player']
    })
    game['current_player'] = game['human_player']
    game['game_over'] = result.get('game_over', False)
    game['winner'] = result.get('winner')
    
    logger.info(f"[GAME] AI move: ({ai_move_x}, {ai_move_y}) by {game['moves'][-1]['player']}, "
                f"new_current={game['current_player']}, game_over={game['game_over']}, "
                f"winner={game['winner']}, total_moves={len(game['moves'])}")
    
    return jsonify({
        'board': result['board'],
        'current_player': game['current_player'],
        'move': result['move'],
        'stats': result.get('stats'),
        'game_over': game['game_over'],
        'winner': game['winner']
    })


@app.route('/api/game_state/<game_id>', methods=['GET'])
def game_state(game_id):
    logger.info(f"[API] GET /api/game_state/{game_id}")
    
    if game_id not in games:
        logger.warning(f"[API] game_state: Game not found, game_id={game_id}")
        return jsonify({'error': 'Game not found'}), 404
    
    game = games[game_id]
    
    result = call_cpp_engine(
        'get_state',
        game['win_length'],
        game['moves'],
        game['current_player']
    )
    
    if not result.get('success'):
        logger.error(f"[API] game_state failed: {result.get('error')}")
        return jsonify(result), 500
    
    logger.debug(f"[API] game_state: returning state for game_id={game_id}")
    
    return jsonify({
        'board': result['board'],
        'current_player': game['current_player'],
        'moves': game['moves'],
        'game_over': game['game_over'],
        'winner': game['winner'],
        'win_length': game['win_length'],
        'human_player': game['human_player']
    })


@app.route('/api/reset_game/<game_id>', methods=['POST'])
def reset_game(game_id):
    logger.info(f"[API] POST /api/reset_game/{game_id}")
    
    if game_id not in games:
        logger.warning(f"[API] reset_game: Game not found, game_id={game_id}")
        return jsonify({'error': 'Game not found'}), 404
    
    game = games[game_id]
    logger.info(f"[GAME] Resetting game_id={game_id}")
    
    game['moves'] = []
    game['current_player'] = 'X'
    game['game_over'] = False
    game['winner'] = None
    
    state_result = call_cpp_engine('get_state', game['win_length'], [], 'X')
    
    if not state_result.get('success'):
        logger.error(f"[API] reset_game failed: {state_result.get('error')}")
        return jsonify(state_result), 500
    
    logger.info(f"[GAME] Game reset complete, game_id={game_id}")
    
    return jsonify({
        'board': state_result.get('board', {'cells': [], 'bbox': {'min_x': 0, 'max_x': 0, 'min_y': 0, 'max_y': 0}}),
        'current_player': 'X',
        'moves': [],
        'game_over': False,
        'winner': None
    })


if __name__ == '__main__':
    print("=" * 50)
    print("Infinite Tic-Tac-Toe Web Interface")
    print("=" * 50)
    print(f"Engine Path: {WEB_CLI_PATH}")
    print(f"Exists: {os.path.exists(WEB_CLI_PATH)}")
    print(f"Log File: {LOG_FILE}")
    print()
    print("Starting server on http://localhost:5000")
    print("=" * 50)
    
    logger.info(f"Engine Path: {WEB_CLI_PATH}, Exists: {os.path.exists(WEB_CLI_PATH)}")
    logger.info("Starting Flask server on http://localhost:5000")
    
    app.run(debug=True, host='0.0.0.0', port=5000)

