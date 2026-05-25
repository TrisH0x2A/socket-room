import { writable } from 'svelte/store';
const KEY_TOKEN = 'sr_token';
const KEY_USER  = 'sr_user';
function createAuthStore() {
  let initial = { user: null, token: '' };
  try {
    const saved = sessionStorage.getItem(KEY_USER);
    const token = sessionStorage.getItem(KEY_TOKEN);
    if (saved && token) {
      initial = { user: JSON.parse(saved), token };
    }
  } catch (_) {}
  const { subscribe, set, update } = writable(initial);
  return {
    subscribe,
    login(user, token) {
      const state = { user, token };
      try {
        sessionStorage.setItem(KEY_USER,  JSON.stringify(user));
        sessionStorage.setItem(KEY_TOKEN, token);
      } catch (_) {}
      set(state);
    },
    logout() {
      try {
        sessionStorage.removeItem(KEY_USER);
        sessionStorage.removeItem(KEY_TOKEN);
      } catch (_) {}
      set({ user: null, token: '' });
    },
  };
}
export const auth = createAuthStore();
