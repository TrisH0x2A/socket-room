<script>
  import { createEventDispatcher } from 'svelte';
  import { api } from '../services/api.js';
  export let show = false;
  const dispatch = createEventDispatcher();
  let activeTab = 'password';
  let oldPassword = '';
  let newPassword = '';
  let confirmPassword = '';
  let currentPassword = '';
  let newUsername = '';
  let errorMsg = '';
  let successMsg = '';
  let loading = false;
  function close() {
    oldPassword = '';
    newPassword = '';
    confirmPassword = '';
    currentPassword = '';
    newUsername = '';
    errorMsg = '';
    successMsg = '';
    activeTab = 'password';
    dispatch('close');
  }
  async function handlePasswordSubmit() {
    errorMsg = '';
    successMsg = '';
    if (!oldPassword || !newPassword || !confirmPassword) {
      errorMsg = 'All fields are required.';
      return;
    }
    if (newPassword !== confirmPassword) {
      errorMsg = 'New passwords do not match.';
      return;
    }
    if (newPassword.length < 6) {
      errorMsg = 'Password must be at least 6 characters.';
      return;
    }
    loading = true;
    try {
      await api.changePassword(oldPassword, newPassword);
      successMsg = 'Password changed successfully.';
      oldPassword = '';
      newPassword = '';
      confirmPassword = '';
    } catch (e) {
      errorMsg = e.message || 'Failed to change password.';
    } finally {
      loading = false;
    }
  }
  async function handleUsernameSubmit() {
    errorMsg = '';
    successMsg = '';
    if (!currentPassword || !newUsername) {
      errorMsg = 'All fields are required.';
      return;
    }
    if (newUsername.length < 3 || newUsername.length > 30) {
      errorMsg = 'Username must be 3-30 characters.';
      return;
    }
    loading = true;
    try {
      await api.changeUsername(currentPassword, newUsername);
      successMsg = 'Username changed successfully. Please refresh to see changes everywhere.';
      currentPassword = '';
      newUsername = '';
    } catch (e) {
      errorMsg = e.message || 'Failed to change username.';
    } finally {
      loading = false;
    }
  }
  function setTab(tab) {
    activeTab = tab;
    errorMsg = '';
    successMsg = '';
  }
  function handleKeydown(e) {
    if (e.key === 'Escape') close();
  }
</script>
<svelte:window on:keydown={handleKeydown} />
{#if show}
  <div class="modal-overlay" on:click={close}>
    <div class="modal-content" on:click|stopPropagation>
      <div class="modal-header">
        <h2>[ SETTINGS ]</h2>
        <button class="close-btn" on:click={close}>✕</button>
      </div>
      <div class="tabs">
        <button class="tab-btn" class:active={activeTab === 'password'} on:click={() => setTab('password')}>PASSWORD</button>
        <button class="tab-btn" class:active={activeTab === 'username'} on:click={() => setTab('username')}>USERNAME</button>
      </div>
      <div class="modal-body">
        {#if activeTab === 'password'}
          <form on:submit|preventDefault={handlePasswordSubmit}>
            <div class="form-group">
              <label for="oldPassword">Current Password</label>
              <input type="password" id="oldPassword" bind:value={oldPassword} autocomplete="current-password" />
            </div>
            <div class="form-group">
              <label for="newPassword">New Password</label>
              <input type="password" id="newPassword" bind:value={newPassword} autocomplete="new-password" />
            </div>
            <div class="form-group">
              <label for="confirmPassword">Confirm Password</label>
              <input type="password" id="confirmPassword" bind:value={confirmPassword} autocomplete="new-password" />
            </div>
            {#if errorMsg} <div class="error-msg">{errorMsg}</div> {/if}
            {#if successMsg} <div class="success-msg">{successMsg}</div> {/if}
            <div class="modal-footer">
              <button type="submit" class="submit-btn" disabled={loading}>
                {loading ? 'SAVING...' : 'CHANGE PASSWORD'}
              </button>
            </div>
          </form>
        {:else}
          <form on:submit|preventDefault={handleUsernameSubmit}>
            <div class="form-group">
              <label for="newUsername">New Username</label>
              <input type="text" id="newUsername" bind:value={newUsername} autocomplete="off" />
            </div>
            <div class="form-group">
              <label for="currentPassword">Current Password</label>
              <input type="password" id="currentPassword" bind:value={currentPassword} autocomplete="current-password" />
            </div>
            {#if errorMsg} <div class="error-msg">{errorMsg}</div> {/if}
            {#if successMsg} <div class="success-msg">{successMsg}</div> {/if}
            <div class="modal-footer">
              <button type="submit" class="submit-btn" disabled={loading}>
                {loading ? 'SAVING...' : 'CHANGE USERNAME'}
              </button>
            </div>
          </form>
        {/if}
      </div>
    </div>
  </div>
{/if}
<style>
  .modal-overlay {
    position: fixed;
    inset: 0;
    background: rgba(0, 0, 0, 0.7);
    backdrop-filter: blur(4px);
    display: flex;
    align-items: center;
    justify-content: center;
    z-index: 1000;
  }
  .modal-content {
    background: var(--bg-alt);
    border: 1px solid var(--border);
    width: 90%;
    max-width: 400px;
    box-shadow: 0 8px 32px rgba(0, 0, 0, 0.8), 0 0 0 1px rgba(0, 255, 65, 0.1);
    animation: slideUp 0.3s cubic-bezier(0.16, 1, 0.3, 1);
  }
  .modal-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 1rem 1.25rem;
    border-bottom: 1px solid var(--border);
    background: rgba(0, 255, 65, 0.05);
  }
  .modal-header h2 {
    margin: 0;
    font-size: 14px;
    color: var(--green);
    letter-spacing: 0.1em;
  }
  .close-btn {
    background: none;
    border: none;
    color: var(--text-muted);
    font-size: 16px;
    cursor: pointer;
    transition: color 0.2s;
  }
  .close-btn:hover {
    color: var(--green);
  }
  .tabs {
    display: flex;
    border-bottom: 1px solid var(--border);
    background: rgba(0, 0, 0, 0.2);
  }
  .tab-btn {
    flex: 1;
    background: transparent;
    border: none;
    border-bottom: 2px solid transparent;
    color: var(--text-muted);
    padding: 10px 0;
    font-family: inherit;
    font-size: 11px;
    font-weight: bold;
    letter-spacing: 0.1em;
    cursor: pointer;
    transition: all 0.2s;
  }
  .tab-btn:hover {
    color: var(--text-primary);
  }
  .tab-btn.active {
    color: var(--green);
    border-bottom-color: var(--green);
    background: rgba(0, 255, 65, 0.03);
  }
  .modal-body {
    padding: 1.25rem;
  }
  .form-group {
    margin-bottom: 1rem;
  }
  .form-group label {
    display: block;
    margin-bottom: 0.5rem;
    font-size: 12px;
    color: var(--text-muted);
    text-transform: uppercase;
    letter-spacing: 0.05em;
  }
  .form-group input {
    width: 100%;
    background: var(--bg);
    border: 1px solid var(--border);
    color: var(--text-primary);
    padding: 0.75rem;
    font-family: inherit;
    font-size: 14px;
    transition: all 0.2s ease;
  }
  .form-group input:focus {
    outline: none;
    border-color: var(--green);
    box-shadow: 0 0 8px rgba(0, 255, 65, 0.2);
  }
  .error-msg, .success-msg {
    padding: 0.75rem;
    margin-bottom: 1rem;
    font-size: 12px;
    border-left: 3px solid;
  }
  .error-msg {
    background: rgba(255, 0, 0, 0.1);
    border-color: red;
    color: #ff5555;
  }
  .success-msg {
    background: rgba(0, 255, 65, 0.1);
    border-color: var(--green);
    color: var(--green);
  }
  .modal-footer {
    margin-top: 1.5rem;
    display: flex;
    justify-content: flex-end;
  }
  .submit-btn {
    background: var(--green);
    color: var(--bg);
    border: none;
    padding: 0.75rem 1.5rem;
    font-family: inherit;
    font-weight: bold;
    cursor: pointer;
    text-transform: uppercase;
    letter-spacing: 0.1em;
    transition: all 0.2s ease;
  }
  .submit-btn:hover:not(:disabled) {
    box-shadow: 0 0 12px rgba(0, 255, 65, 0.6);
    transform: translateY(-1px);
  }
  .submit-btn:active:not(:disabled) {
    transform: translateY(0);
  }
  .submit-btn:disabled {
    opacity: 0.5;
    cursor: not-allowed;
  }
  @keyframes slideUp {
    from {
      opacity: 0;
      transform: translateY(20px);
    }
    to {
      opacity: 1;
      transform: translateY(0);
    }
  }
</style>
