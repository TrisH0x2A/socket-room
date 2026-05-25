import { connected } from '../stores/chat.js';
import { addMessage, socketError } from '../stores/chat.js';
const listeners = new Map();
function emit(event, data) {
  (listeners.get(event) || []).forEach(fn => fn(data));
}
export function socketOn(event, cb) {
  const arr = listeners.get(event) || [];
  listeners.set(event, [...arr, cb]);
  return () => {
    const cur = listeners.get(event) || [];
    listeners.set(event, cur.filter(fn => fn !== cb));
  };
}
let ws           = null;
let token        = '';
let manualClose  = false;
let reconnectTimer = null;
let attempt      = 0;           
const MAX_DELAY  = 30_000;      
function backoffDelay() {
  return Math.min(1000 * Math.pow(2, attempt), MAX_DELAY);
}
function _connect() {
  if (ws) return;                                   
  if (manualClose) return;
  const proto = location.protocol === 'https:' ? 'wss' : 'ws';
  let baseUrl = import.meta.env.VITE_WS_URL;
  if (!baseUrl) {
    baseUrl = `${proto}:
  }
  const url = `${baseUrl}?token=${encodeURIComponent(token)}`;
  try {
    ws = new WebSocket(url);
  } catch (e) {
    console.warn('[socket] WebSocket constructor failed:', e);
    ws = null;
    _scheduleReconnect();
    return;
  }
  ws.onopen = () => {
    attempt = 0;                                    
    connected.set(true);
    emit('open', null);
  };
  ws.onmessage = (e) => {
    try {
      const msg = JSON.parse(e.data);
      emit('message', msg);
      if (msg.type === 'message' || msg.type === 'history') {
        addMessage(msg);
      }
      if (msg.type === 'delete') {
        import('../stores/chat.js').then(module => {
          module.deleteMessage(msg.id);
        });
      }
      if (msg.type === 'error') {
        console.warn('[socket] Server error:', msg.message);
        if (msg.message) {
          socketError.set(msg.message);
          setTimeout(() => socketError.set(null), 4000);
        }
        if (msg.message && (
          msg.message.includes('authenticated') ||
          msg.message.includes('session')
        )) {
          manualClose = true;   
        }
      }
    } catch (_) {}
  };
  ws.onerror = () => {
    try { ws?.close(); } catch (_) {}
  };
  ws.onclose = () => {
    connected.set(false);
    ws = null;
    emit('close', null);
    if (!manualClose) {
      _scheduleReconnect();
    }
  };
}
function _scheduleReconnect() {
  if (reconnectTimer) return;
  const delay = backoffDelay();
  attempt++;
  reconnectTimer = setTimeout(() => {
    reconnectTimer = null;
    _connect();
  }, delay);
}
export function socketConnect(sessionToken) {
  token       = sessionToken || '';
  manualClose = false;
  attempt     = 0;
  if (reconnectTimer) { clearTimeout(reconnectTimer); reconnectTimer = null; }
  if (ws) { ws.onclose = null; ws.close(); ws = null; }
  connected.set(false);
  _connect();
}
export function socketDisconnect() {
  manualClose = true;
  if (reconnectTimer) { clearTimeout(reconnectTimer); reconnectTimer = null; }
  if (ws) { try { ws.close(); } catch (_) {} ws = null; }
  connected.set(false);
}
export function socketJoinRoom(roomId) {
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({ type: 'join', room_id: roomId }));
  }
}
export function socketSendMessage(roomId, text) {
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({ type: 'message', room_id: roomId, text }));
  }
}
export function socketDeleteMessage(msgId) {
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({ type: 'delete', id: msgId }));
  }
}
