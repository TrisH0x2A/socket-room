<script>
  import { messages, currentRoom } from '../stores/chat.js';
  import { socketDeleteMessage } from '../services/socket.js';
  import { onMount, afterUpdate, tick } from 'svelte';
  export let currentUser = null;
  function scrollAction(node, messages) {
    return {
      update(newMessages) {
        if (autoScroll && node) {
          setTimeout(() => {
            node.scrollTop = node.scrollHeight;
          }, 0);
        }
      }
    };
  }
  function onScroll(e) {
    const node = e.target;
    if (!node) return;
    const nearBottom = node.scrollHeight - node.scrollTop - node.clientHeight <= 82;
    autoScroll = nearBottom;
  }
  let feedEl;
  let autoScroll = true;
  function formatTime(ts) {
    const d = new Date(ts * 1000);
    const h = String(d.getHours()).padStart(2, '0');
    const m = String(d.getMinutes()).padStart(2, '0');
    return `${h}:${m}`;
  }
  function formatDate(ts) {
    return new Date(ts * 1000).toLocaleDateString('en-US', {
      month: 'short', day: 'numeric'
    });
  }
  $: grouped = groupMessages($messages.filter(m => !$currentRoom || m.room_id === $currentRoom?.id));
  function groupMessages(msgs) {
    const groups = [];
    for (const msg of msgs) {
      const last = groups[groups.length - 1];
      if (last && last.username === msg.username && msg.created_at - last.lastTs < 300) {
        last.messages.push(msg);
        last.lastTs = msg.created_at;
      } else {
        groups.push({ username: msg.username, color: msg.color, messages: [msg], lastTs: msg.created_at });
      }
    }
    return groups;
  }
  function isSelf(username) {
    return currentUser && username === currentUser.username;
  }
  function getInitial(name) {
    return (name || '?')[0].toUpperCase();
  }
  let openMenuId = null;
  let deleteConfirmId = null;
  function toggleMenu(msgId) {
    openMenuId = openMenuId === msgId ? null : msgId;
    deleteConfirmId = null;
  }
  function closeMenu() {
    openMenuId = null;
    deleteConfirmId = null;
  }
  function triggerDelete(msg) {
    if (deleteConfirmId === msg.id) {
      socketDeleteMessage(msg.id);
      closeMenu();
    } else {
      deleteConfirmId = msg.id;
    }
  }
</script>
<svelte:window on:click={closeMenu} />
<div class="feed" use:scrollAction={$messages} on:scroll={onScroll}>
  {#if !$currentRoom}
    <div class="empty-state">
      <div class="ascii-art">
        <pre>
  ╔══════════════════╗
  ║   SELECT A ROOM  ║
  ╚══════════════════╝</pre>
      </div>
      <p>Choose a channel to start chatting</p>
    </div>
  {:else if grouped.length === 0}
    <div class="empty-state">
      <div class="ascii-art">
        <pre>
  [ NO MESSAGES YET ]
  Be the first to say
  something in #{$currentRoom.name}</pre>
      </div>
    </div>
  {:else}
    <div class="messages-inner">
      {#each grouped as group, gi}
        <div class="msg-group fade-in" class:self={isSelf(group.username)}>
          <div class="msg-avatar {group.color}">{getInitial(group.username)}</div>
          <div class="msg-content">
            <div class="msg-header">
              <span class="msg-username {group.color}">
                {group.username}
              </span>
              <span class="msg-time">{formatTime(group.messages[0].created_at)}</span>
            </div>
            {#each group.messages as msg}
              <div class="msg-row">
                <div class="msg-text">{msg.text}</div>
                {#if isSelf(group.username)}
                  <div class="msg-menu">
                    <button class="msg-menu-btn {openMenuId === msg.id ? 'active' : ''}" on:click|stopPropagation={() => toggleMenu(msg.id)}>⋮</button>
                    {#if openMenuId === msg.id}
                      <div class="msg-dropdown">
                        <button class:confirm-state={deleteConfirmId === msg.id} on:click|stopPropagation={() => triggerDelete(msg)}>
                          {deleteConfirmId === msg.id ? 'Are you sure?' : 'Delete'}
                        </button>
                      </div>
                    {/if}
                  </div>
                {/if}
              </div>
            {/each}
          </div>
        </div>
      {/each}
    </div>
  {/if}
</div>
<style>
  .feed {
    flex: 1;
    overflow-y: auto;
    padding: 16px 0;
    display: flex;
    flex-direction: column;
  }
  .messages-inner {
    display: flex;
    flex-direction: column;
    gap: 2px;
    padding: 0 16px;
    margin-top: auto;
  }
  .msg-group {
    display: flex;
    gap: 12px;
    padding: 5px 8px;
    border-radius: 2px;
    transition: background 0.1s;
    align-items: flex-start;
  }
  .msg-group:hover { background: rgba(0,255,65,0.03); }
  .msg-avatar {
    width: 32px;
    height: 32px;
    min-width: 32px;
    background: var(--bg-hover);
    border: 1px solid var(--border);
    color: var(--text-dim);
    font-size: 13px;
    font-weight: 700;
    display: flex;
    align-items: center;
    justify-content: center;
    margin-top: 2px;
  }
  .msg-avatar.c1 { color: var(--c1); border-color: rgba(0,255,255,0.2); }
  .msg-avatar.c2 { color: var(--c2); border-color: rgba(255,0,255,0.2); }
  .msg-avatar.c3 { color: var(--c3); border-color: rgba(0,255,65,0.2); }
  .msg-avatar.c4 { color: var(--c4); border-color: rgba(255,215,0,0.2); }
  .msg-avatar.c5 { color: var(--c5); border-color: rgba(176,82,255,0.2); }
  .msg-avatar.c6 { color: var(--c6); border-color: rgba(255,68,68,0.2); }
  .msg-content { flex: 1; min-width: 0; }
  .msg-header {
    display: flex;
    align-items: baseline;
    gap: 10px;
    margin-bottom: 3px;
  }
  .msg-username {
    font-size: 12px;
    font-weight: 700;
    color: var(--text-muted);
    letter-spacing: 0.05em;
  }
  .msg-username.c1 { color: var(--c1); text-shadow: 0 0 8px rgba(0,255,255,0.3); }
  .msg-username.c2 { color: var(--c2); text-shadow: 0 0 8px rgba(255,0,255,0.3); }
  .msg-username.c3 { color: var(--c3); text-shadow: 0 0 8px rgba(0,255,65,0.3); }
  .msg-username.c4 { color: var(--c4); text-shadow: 0 0 8px rgba(255,215,0,0.3); }
  .msg-username.c5 { color: var(--c5); text-shadow: 0 0 8px rgba(176,82,255,0.3); }
  .msg-username.c6 { color: var(--c6); text-shadow: 0 0 8px rgba(255,68,68,0.3); }
  .msg-time {
    font-size: 10px;
    color: var(--text-dim);
    letter-spacing: 0.05em;
  }
  .msg-text {
    font-size: 13px;
    color: var(--text-primary);
    line-height: 1.55;
    word-break: break-word;
    white-space: pre-wrap;
    padding: 1px 0;
  }
  .empty-state {
    flex: 1;
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    gap: 16px;
    color: var(--text-muted);
  }
  .ascii-art pre {
    color: var(--green-muted);
    font-size: 12px;
    line-height: 1.4;
    text-align: center;
  }
  .empty-state p {
    font-size: 11px;
    color: var(--text-dim);
    letter-spacing: 0.05em;
  }
  @keyframes fadeIn {
    from { opacity: 0; transform: translateY(4px); }
    to   { opacity: 1; transform: translateY(0); }
  }
  .fade-in { animation: fadeIn 0.2s ease; }
  .msg-row {
    display: flex;
    justify-content: space-between;
    align-items: flex-start;
    gap: 8px;
    position: relative;
    width: 100%;
  }
  .msg-menu { position: relative; }
  .msg-menu-btn {
    opacity: 0.4;
    background: transparent;
    border: none;
    color: var(--text-muted);
    cursor: pointer;
    font-size: 16px;
    padding: 0 8px;
    font-weight: bold;
    flex-shrink: 0;
    transition: opacity 0.2s, color 0.2s;
  }
  .msg-row:hover .msg-menu-btn { opacity: 1; color: var(--text-primary); }
  .msg-menu-btn:hover, .msg-menu-btn.active { opacity: 1 !important; color: var(--text-primary); }
  .msg-dropdown {
    position: absolute;
    right: 100%;
    top: 0;
    background: var(--bg-panel);
    border: 1px solid var(--border);
    border-radius: 4px;
    box-shadow: 0 4px 12px rgba(0,0,0,0.2);
    z-index: 10;
  }
  .msg-dropdown button {
    display: block;
    width: 100%;
    padding: 8px 16px;
    background: transparent;
    border: none;
    color: var(--text-primary);
    cursor: pointer;
    font-size: 12px;
    text-align: left;
    white-space: nowrap;
    transition: background 0.1s, color 0.1s;
  }
  .msg-dropdown button:hover {
    background: rgba(255, 68, 68, 0.1);
    color: #ff4444;
  }
  .msg-dropdown button.confirm-state {
    background: rgba(255, 68, 68, 0.15);
    color: #ff4444;
    font-weight: bold;
  }
</style>
