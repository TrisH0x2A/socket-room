<script>
  import { rooms, currentRoom } from '../stores/chat.js';
  export let onSelect = () => {};
  export let onLogout = () => {};
  export let onSettings = () => {};
  const ROOM_ICONS = {
    'system-programming': '⚙',
    'backend':  '◈',
    'frontend': '◇',
    'chill-zone': '~',
  };
  function icon(name) {
    return ROOM_ICONS[name] || '#';
  }
</script>
<aside class="room-list">
  <nav class="channels">
    {#each $rooms as room (room.id)}
      <button
        class="channel-btn"
        class:active={$currentRoom?.id === room.id}
        on:click={() => onSelect(room)}
      >
        <span class="channel-icon">{icon(room.name)}</span>
        <span class="channel-name">{room.name}</span>
        {#if $currentRoom?.id === room.id}
          <span class="active-marker">◀</span>
        {/if}
      </button>
    {/each}
  </nav>
  <div class="sidebar-footer">
    <div class="footer-actions">
      <div class="status-row">
        <div class="status-dot"></div>
        <span>ONLINE</span>
      </div>
      <div class="btn-group">
        <button class="action-btn" on:click={onSettings} title="Settings">[SET]</button>
        <button class="action-btn exit-btn" on:click={onLogout} title="Logout">[EXIT]</button>
      </div>
    </div>
  </div>
</aside>
<style>
  .room-list {
    width: 100%;
    min-width: 200px;
    background: var(--bg-panel);
    border-right: 1px solid var(--border);
    display: flex;
    flex-direction: column;
    height: 100%;
    overflow: hidden;
  }
  .channels {
    flex: 1;
    overflow-y: auto;
    padding: 4px 6px;
  }
  .channel-btn {
    width: 100%;
    display: flex;
    align-items: center;
    gap: 8px;
    padding: 7px 10px;
    background: transparent;
    border: none;
    color: var(--text-muted);
    cursor: pointer;
    font-family: inherit;
    font-size: 12px;
    text-align: left;
    border-radius: 2px;
    transition: background 0.1s, color 0.1s;
    position: relative;
  }
  .channel-btn:hover {
    background: var(--bg-hover);
    color: var(--text-primary);
  }
  .channel-btn.active {
    background: rgba(0, 255, 65, 0.07);
    color: var(--green);
    border-left: 2px solid var(--green);
    padding-left: 8px;
  }
  .channel-icon {
    font-size: 11px;
    opacity: 0.7;
    width: 14px;
    text-align: center;
    flex-shrink: 0;
  }
  .channel-name {
    flex: 1;
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
    letter-spacing: 0.03em;
  }
  .active-marker {
    font-size: 9px;
    color: var(--green-dim);
  }
  .sidebar-footer {
    padding: 0 14px;
    height: 54px;
    border-top: 1px solid var(--border);
    display: flex;
    flex-direction: column;
    justify-content: center;
  }
  .footer-actions {
    display: flex;
    justify-content: space-between;
    align-items: center;
  }
  .btn-group {
    display: flex;
    gap: 10px;
  }
  .action-btn {
    background: transparent;
    border: none;
    font-family: inherit;
    font-size: 11px;
    font-weight: 700;
    color: var(--text-muted);
    cursor: pointer;
    letter-spacing: 0.1em;
    transition: color 0.1s;
    padding: 2px 0;
  }
  .action-btn:hover { color: var(--green); }
  .action-btn.exit-btn:hover { color: #ff4444; }
  .status-row {
    display: flex;
    align-items: center;
    gap: 7px;
    font-size: 10px;
    color: var(--green-dim);
    letter-spacing: 0.08em;
  }
  .status-dot {
    width: 6px;
    height: 6px;
    border-radius: 50%;
    background: var(--green);
    box-shadow: 0 0 6px var(--green);
    animation: pulse 2s ease-in-out infinite;
  }
  @keyframes pulse {
    0%, 100% { opacity: 1; }
    50% { opacity: 0.4; }
  }
</style>
