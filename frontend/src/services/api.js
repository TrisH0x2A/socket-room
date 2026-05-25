const BASE = import.meta.env.VITE_API_URL || '/api';
async function request(method, path, body) {
  const opts = {
    method,
    credentials: 'include',
    headers: { 'Content-Type': 'application/json' },
  };
  if (body) opts.body = JSON.stringify(body);
  let res;
  try {
    res = await fetch(BASE + path, opts);
  } catch (err) {
    err.isOffline = true;
    throw err;
  }
  if (res.status === 502 || res.status === 503 || res.status === 504) {
    const err = new Error(`HTTP ${res.status}`);
    err.isOffline = true;
    throw err;
  }
  const data = await res.json().catch(() => ({}));
  if (!res.ok) throw new Error(data.error || `HTTP ${res.status}`);
  return data;
}
export const api = {
  register: (username, password)  => request('POST', '/register', { username, password }),
  login:    (username, password)  => request('POST', '/login',    { username, password }),
  logout:   ()                    => request('POST', '/logout'),
  me:       ()                    => request('GET',  '/me'),
  rooms:    ()                    => request('GET',  '/rooms'),
  messages: (room_id, limit = 50) => request('GET',  `/messages?room_id=${room_id}&limit=${limit}`),
  changePassword: (old_password, new_password) => request('POST', '/change_password', { old_password, new_password }),
  changeUsername: (current_password, new_username) => request('POST', '/change_username', { current_password, new_username }),
};
