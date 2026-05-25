<script>
  import { onMount, onDestroy } from 'svelte';
  import { get } from 'svelte/store';
  import { rooms, currentRoom, connected, clearMessages, socketError } from '../stores/chat.js';
  import { socketConnect, socketDisconnect, socketJoinRoom, socketOn } from '../services/socket.js';
  import RoomList    from '../components/RoomList.svelte';
  import MessageFeed from '../components/MessageFeed.svelte';
  import MessageInput from '../components/MessageInput.svelte';
  import SettingsModal from '../components/SettingsModal.svelte';
  import { api } from '../services/api.js';
  export let user    = null;
  export let onLogout = () => {};
  export let token   = '';
  let error = '';
  let showSettings = false;
  let showSidebarMobile = false;
  let lightMode = false;
  function toggleTheme() {
    lightMode = !lightMode;
    if (lightMode) {
      document.documentElement.classList.add('light-theme');
      localStorage.setItem('theme', 'light');
    } else {
      document.documentElement.classList.remove('light-theme');
      localStorage.setItem('theme', 'dark');
    }
  }
  async function selectRoom(room) {
    clearMessages();
    currentRoom.set(room);
    socketJoinRoom(room.id);
    showSidebarMobile = false; 
  }
  async function logout() {
    socketDisconnect();
    await api.logout().catch(() => {});
    onLogout();
  }
  onMount(() => { console.log("[Chat] onMount started");
    lightMode = localStorage.getItem('theme') === 'light';
    if (lightMode) {
      document.documentElement.classList.add('light-theme');
    }
    const unsub = socketOn('open', () => {
      const curRoom  = get(currentRoom);
      const roomList = get(rooms);
      if (!curRoom && roomList.length > 0) {
        selectRoom(roomList[0]);
      }
    });
    (async () => {
      try {
        const roomList = await api.rooms();
        rooms.set(roomList);
        socketConnect(token);
      } catch (e) {
        error = 'Failed to load rooms: ' + e.message;
      }
    })();
    return () => {
      unsub();
      socketDisconnect();
    };
  });
</script>
<div class="chat-layout">
  <header class="topbar">
    <div class="topbar-left">
      <button class="mobile-menu-btn" on:click={() => showSidebarMobile = !showSidebarMobile}>
        <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
          <line x1="3" y1="12" x2="21" y2="12"></line>
          <line x1="3" y1="6" x2="21" y2="6"></line>
          <line x1="3" y1="18" x2="21" y2="18"></line>
        </svg>
      </button>
      <span class="logo-sm scan-text">
        <span style="color:var(--green)" class="glow-subtle">SOCKET</span><span style="color:var(--text-muted)">ROOM</span>
      </span>
    </div>
    <div class="topbar-right">
      <a href="https://github.com/TrisH0x2A/socket-room" target="_blank" rel="noreferrer" class="icon-link" title="GitHub">
        <svg width="18" height="18" viewBox="0 0 24 24" fill="currentColor">
          <path d="M12 0c-6.626 0-12 5.373-12 12 0 5.302 3.438 9.8 8.207 11.387.599.111.793-.261.793-.577v-2.234c-3.338.726-4.033-1.416-4.033-1.416-.546-1.387-1.333-1.756-1.333-1.756-1.089-.745.083-.729.083-.729 1.205.084 1.839 1.237 1.839 1.237 1.07 1.834 2.807 1.304 3.492.997.107-.775.418-1.305.762-1.604-2.665-.305-5.467-1.334-5.467-5.931 0-1.311.469-2.381 1.236-3.221-.124-.303-.535-1.524.117-3.176 0 0 1.008-.322 3.301 1.23.957-.266 1.983-.399 3.003-.404 1.02.005 2.047.138 3.006.404 2.291-1.552 3.297-1.23 3.297-1.23.653 1.653.242 2.874.118 3.176.77.84 1.235 1.911 1.235 3.221 0 4.609-2.807 5.624-5.479 5.921.43.372.823 1.102.823 2.222v3.293c0 .319.192.694.801.576 4.765-1.589 8.199-6.086 8.199-11.386 0-6.627-5.373-12-12-12z"/>
        </svg>
      </a>
      <button class="icon-btn" on:click={toggleTheme} title="Toggle Theme">
        {#if lightMode}
          <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M21 12.79A9 9 0 1 1 11.21 3 7 7 0 0 0 21 12.79z"></path></svg>
        {:else}
          <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="5"></circle><line x1="12" y1="1" x2="12" y2="3"></line><line x1="12" y1="21" x2="12" y2="23"></line><line x1="4.22" y1="4.22" x2="5.64" y2="5.64"></line><line x1="18.36" y1="18.36" x2="19.78" y2="19.78"></line><line x1="1" y1="12" x2="3" y2="12"></line><line x1="21" y1="12" x2="23" y2="12"></line><line x1="4.22" y1="19.78" x2="5.64" y2="18.36"></line><line x1="18.36" y1="5.64" x2="19.78" y2="4.22"></line></svg>
        {/if}
      </button>
    </div>
  </header>
  {#if showSidebarMobile}
    <div class="sidebar-overlay" on:click={() => showSidebarMobile = false}></div>
  {/if}
  <div class="main">
    <div class="sidebar-container" class:mobile-open={showSidebarMobile}>
      <RoomList 
        onSelect={selectRoom} 
        onLogout={logout} 
        onSettings={() => showSettings = true} 
      />
    </div>
    <div class="chat-area">
      {#if $currentRoom}
        <div class="channel-header">
          <span class="ch-icon">◈</span>
          <span class="ch-name">#{$currentRoom.name}</span>
        </div>
      {/if}
      {#if error}
        <div class="error-banner">[!] {error}</div>
      {/if}
      {#if $socketError}
        <div class="error-banner" style="background: rgba(255,68,68,0.15); color: #ff4444; border-bottom: 1px solid #ff4444; padding: 8px 16px; font-weight: bold; text-align: center;">[!] {$socketError}</div>
      {/if}
      <MessageFeed currentUser={user} />
      <MessageInput />
    </div>
  </div>
</div>
<SettingsModal show={showSettings} on:close={() => showSettings = false} />
<style>
  .chat-layout {
    display: flex;
    flex-direction: column;
    height: 100vh;
    width: 100%;
    overflow: hidden;
  }
  .topbar {
    height: 42px;
    min-height: 42px;
    background: var(--bg-panel);
    border-bottom: 1px solid var(--border);
    display: flex;
    align-items: center;
    justify-content: space-between;
    padding: 0 16px;
    gap: 16px;
    z-index: 10;
  }
  .topbar-left {
    display: flex;
    align-items: center;
    gap: 10px;
    flex: 1;
    min-width: 0;
  }
  .logo-sm {
    font-size: 16px;
    font-weight: 700;
    letter-spacing: 0.08em;
    flex-shrink: 0;
  }
  .topbar-right {
    display: flex;
    align-items: center;
    gap: 14px;
    flex-shrink: 0;
  }
  .icon-btn, .icon-link {
    background: transparent;
    border: none;
    color: var(--text-muted);
    font-family: inherit;
    font-size: 14px;
    font-weight: bold;
    cursor: pointer;
    text-decoration: none;
    transition: color 0.2s, text-shadow 0.2s;
    display: flex;
    align-items: center;
    justify-content: center;
    line-height: 1;
    padding: 0;
  }
  .icon-btn:hover, .icon-link:hover {
    color: var(--green);
    text-shadow: 0 0 8px rgba(0,255,65,0.4);
  }
  .mobile-menu-btn {
    display: none;
    background: transparent;
    border: none;
    font-family: inherit;
    font-size: 14px;
    font-weight: bold;
    color: var(--green);
    cursor: pointer;
    padding: 0;
  }
  .main {
    display: flex;
    flex: 1;
    overflow: hidden;
    position: relative;
  }
  .sidebar-container {
    display: flex;
    flex-direction: column;
    height: 100%;
    z-index: 5;
    background: var(--bg);
  }
  .chat-area {
    display: flex;
    flex-direction: column;
    flex: 1;
    overflow: hidden;
    min-width: 0;
  }
  .channel-header {
    display: flex;
    align-items: center;
    gap: 8px;
    padding: 8px 18px;
    border-bottom: 1px solid var(--border);
    background: var(--bg-panel);
  }
  .ch-icon  { color: var(--green-muted); font-size: 12px; }
  .ch-name  { font-size: 13px; font-weight: 700; color: var(--text-primary); letter-spacing: 0.04em; }
  .error-banner {
    background: rgba(255,68,68,0.07);
    border-bottom: 1px solid rgba(255,68,68,0.2);
    color: #ff6666;
    font-size: 12px;
    padding: 8px 16px;
    letter-spacing: 0.05em;
  }
  .glow-subtle { text-shadow: 0 0 4px rgba(0,255,65,0.6); }
  @media (max-width: 768px) {
    .mobile-menu-btn {
      display: block;
    }
    .sidebar-container {
      position: absolute;
      left: 0;
      top: 0;
      bottom: 0;
      width: 250px;
      transform: translateX(-100%);
      transition: transform 0.3s cubic-bezier(0.16, 1, 0.3, 1);
      box-shadow: 4px 0 24px rgba(0,0,0,0.5);
    }
    .sidebar-container.mobile-open {
      transform: translateX(0);
    }
    .sidebar-overlay {
      position: absolute;
      inset: 0;
      background: rgba(0,0,0,0.5);
      z-index: 4;
      animation: fadeIn 0.3s;
    }
    @keyframes fadeIn {
      from { opacity: 0; }
      to { opacity: 1; }
    }
  }
</style>
