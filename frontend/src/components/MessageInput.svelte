<script>
  import { connected, currentRoom } from '../stores/chat.js';
  import { socketSendMessage } from '../services/socket.js';
  import { createEventDispatcher } from 'svelte';
  const dispatch = createEventDispatcher();
  let text = '';
  let inputEl;
  let sending = false;
  async function send() {
    const trimmed = text.trim();
    if (!trimmed || !$currentRoom || !$connected || sending) return;
    sending = true;
    socketSendMessage($currentRoom.id, trimmed);
    text = '';
    sending = false;
    inputEl?.focus();
  }
  function onKeydown(e) {
    if (e.key === 'Enter' && !e.shiftKey) {
      e.preventDefault();
      send();
    }
  }
  $: placeholder = $currentRoom
    ? `message #${$currentRoom.name}...`
    : '> select a room first...';
</script>
<div class="input-bar">
  <div class="input-prefix">
    <span class="prompt">{$connected ? '>' : '!'}</span>
  </div>
  <input
    bind:this={inputEl}
    bind:value={text}
    on:keydown={onKeydown}
    class="msg-input"
    type="text"
    {placeholder}
    maxlength="4000"
    disabled={!$connected || !$currentRoom}
    autocomplete="off"
    spellcheck="false"
  />
  <button
    class="send-btn"
    on:click={send}
    disabled={!text.trim() || !$connected || !$currentRoom}
    title="Send (Enter)"
  >
    <span>SEND</span>
    <span class="send-icon">↵</span>
  </button>
</div>
<style>
  .input-bar {
    display: flex;
    align-items: center;
    gap: 0;
    border-top: 1px solid var(--border);
    background: var(--bg-panel);
    padding: 0 14px;
    height: 54px;
  }
  .input-prefix {
    display: flex;
    align-items: center;
    padding: 0 10px 0 0;
  }
  .prompt {
    font-size: 16px;
    font-weight: 700;
    color: var(--green);
    text-shadow: 0 0 8px var(--green);
    font-family: inherit;
  }
  .msg-input {
    flex: 1;
    background: transparent;
    border: none;
    color: var(--text-primary);
    font-family: inherit;
    font-size: 13px;
    padding: 6px 8px;
    outline: none;
    caret-color: var(--green);
    letter-spacing: 0.02em;
  }
  .msg-input::placeholder {
    color: var(--text-dim);
    font-style: italic;
  }
  .msg-input:focus {
  }
  .msg-input:disabled {
    opacity: 0.4;
    cursor: not-allowed;
  }
  .send-btn {
    display: flex;
    align-items: center;
    gap: 6px;
    padding: 6px 14px;
    margin-left: 10px;
    background: transparent;
    border: 1px solid var(--green-muted);
    color: var(--green);
    font-family: inherit;
    font-size: 11px;
    font-weight: 700;
    letter-spacing: 0.1em;
    cursor: pointer;
    transition: all 0.15s;
    flex-shrink: 0;
  }
  .send-btn:hover:not(:disabled) {
    background: rgba(0,255,65,0.1);
    border-color: var(--green);
    box-shadow: 0 0 10px rgba(0,255,65,0.2);
  }
  .send-btn:disabled {
    opacity: 0.3;
    cursor: not-allowed;
  }
  .send-icon {
    font-size: 14px;
    opacity: 0.8;
  }
  @media (max-width: 768px) {
    .input-bar {
      padding: 8px 10px;
      min-height: 48px;
    }
    .msg-input {
      font-size: 14px; 
      padding: 4px 6px;
    }
    .send-btn {
      padding: 6px 10px;
    }
    .send-btn span:first-child {
      display: none; 
    }
  }
</style>
