import { writable, get } from 'svelte/store';
export const rooms        = writable([]);
export const currentRoom  = writable(null);
export const messages     = writable([]);
export const connected    = writable(false);
export const onlineCount  = writable(0);
export const socketError  = writable(null);
let _msgIds = new Set();
export function clearMessages() {
  messages.set([]);
  _msgIds.clear();
}
export function addMessage(msg) {
  if (msg.id && _msgIds.has(msg.id)) return; 
  if (msg.id) _msgIds.add(msg.id);
  messages.update(list => {
    const next = [...list, msg];
    return next.length > 200 ? next.slice(-200) : next;
  });
}
export function deleteMessage(msgId) {
  _msgIds.delete(msgId);
  messages.update(list => list.filter(m => m.id !== msgId));
}
