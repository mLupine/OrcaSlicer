<template>
  <div class="home-view">
    <div class="home-content">
      <section class="hero-section">
        <div class="hero-background" />
        <div class="hero-content">
          <div class="brand">
            <div class="logo-glow" />
            <svg class="logo" viewBox="0 0 48 48" fill="none">
              <path d="M24 4L4 14v20l20 10 20-10V14L24 4z" stroke="currentColor" stroke-width="2" fill="none"/>
              <path d="M24 24L4 14M24 24l20-10M24 24v20" stroke="currentColor" stroke-width="2" opacity="0.5"/>
              <path d="M14 19l10 5 10-5" stroke="currentColor" stroke-width="2"/>
              <circle cx="24" cy="24" r="4" fill="currentColor" opacity="0.8"/>
            </svg>
          </div>
          <div class="hero-text">
            <h1>Welcome to OrcaSlicer</h1>
            <p>Transform your ideas into reality with precision slicing</p>
          </div>
        </div>

        <div class="user-section">
          <div v-if="homeStore.isLoggedIn" class="user-profile">
            <img
              v-if="homeStore.user.userAvatar"
              :src="homeStore.user.userAvatar"
              :alt="homeStore.user.userName"
              class="user-avatar"
              @error="avatarError = true"
            />
            <div v-else class="user-avatar-placeholder">
              <User class="avatar-icon" />
            </div>
            <span class="user-name">{{ homeStore.user.userName || 'User' }}</span>
            <button class="logout-btn" @click="homeStore.logout" title="Sign Out">
              <LogOut class="logout-icon" />
            </button>
          </div>
          <button v-else class="login-btn" @click="homeStore.login">
            <LogIn class="login-icon" />
            <span>Sign In</span>
          </button>
        </div>
      </section>

      <section class="actions-section">
        <div class="action-cards">
          <button class="action-card new-project" @click="homeStore.newProject">
            <div class="card-icon">
              <FilePlus class="icon" />
            </div>
            <div class="card-content">
              <span class="card-title">New Project</span>
              <span class="card-subtitle">Start fresh with a blank canvas</span>
            </div>
            <ChevronRight class="card-arrow" />
          </button>

          <button class="action-card open-project" @click="homeStore.openProject">
            <div class="card-icon">
              <FolderOpen class="icon" />
            </div>
            <div class="card-content">
              <span class="card-title">Open Project</span>
              <span class="card-subtitle">Load 3MF, STL, STEP, or OBJ</span>
            </div>
            <ChevronRight class="card-arrow" />
          </button>
        </div>
      </section>

      <section class="recent-section">
        <div class="section-header">
          <div class="header-left">
            <Clock class="header-icon" />
            <h2>Recent Projects</h2>
            <span v-if="homeStore.hasRecentFiles" class="project-count">
              {{ homeStore.recentFiles.length }}
            </span>
          </div>
          <button
            v-if="homeStore.hasRecentFiles"
            class="clear-btn"
            @click="homeStore.clearAllRecentFiles"
          >
            <Trash2 class="btn-icon" />
            Clear All
          </button>
        </div>

        <div v-if="homeStore.isLoading" class="loading-state">
          <div class="loading-spinner" />
          <span>Loading projects...</span>
        </div>

        <div v-else-if="!homeStore.hasRecentFiles" class="empty-state">
          <div class="empty-icon">
            <FolderOpen class="icon" />
          </div>
          <h3>No recent projects</h3>
          <p>Your recently opened projects will appear here</p>
        </div>

        <div v-else class="recent-grid">
          <RecentFileCard
            v-for="(file, index) in homeStore.recentFiles"
            :key="file.path"
            :file="file"
            :style="{ '--delay': `${index * 50}ms` }"
            @open="homeStore.openRecentFile(file.path)"
            @delete="homeStore.deleteRecentFile(file.path)"
            @explore="homeStore.exploreRecentFile(file.path)"
          />
        </div>
      </section>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue'
import { FilePlus, FolderOpen, ChevronRight, Clock, Trash2, User, LogIn, LogOut } from 'lucide-vue-next'
import { useHomeStore } from '../stores/home'
import RecentFileCard from '../components/RecentFileCard.vue'

const homeStore = useHomeStore()
const avatarError = ref(false)

onMounted(() => {
  homeStore.fetchRecentFiles()
  homeStore.fetchUserInfo()
})
</script>

<style scoped>
.home-view {
  height: 100%;
  background: linear-gradient(180deg, #1a1a1c 0%, #141416 100%);
  overflow-y: auto;
  overflow-x: hidden;
}

.home-content {
  max-width: 1200px;
  margin: 0 auto;
  padding: 32px 40px 48px;
}

.hero-section {
  position: relative;
  padding: 48px 0;
  margin-bottom: 32px;
  border-radius: 24px;
  overflow: hidden;
}

.hero-background {
  position: absolute;
  inset: 0;
  background: linear-gradient(135deg, rgba(59, 130, 246, 0.1) 0%, rgba(147, 51, 234, 0.1) 50%, rgba(236, 72, 153, 0.05) 100%);
  opacity: 0.8;
}

.hero-background::before {
  content: '';
  position: absolute;
  inset: 0;
  background: radial-gradient(ellipse at 30% 20%, rgba(59, 130, 246, 0.15) 0%, transparent 50%);
}

.hero-content {
  position: relative;
  display: flex;
  align-items: center;
  gap: 32px;
  padding: 0 32px;
}

.brand {
  position: relative;
  flex-shrink: 0;
}

.logo-glow {
  position: absolute;
  inset: -20px;
  background: radial-gradient(circle, rgba(59, 130, 246, 0.3) 0%, transparent 70%);
  filter: blur(20px);
  animation: pulse 3s ease-in-out infinite;
}

@keyframes pulse {
  0%, 100% { opacity: 0.5; transform: scale(1); }
  50% { opacity: 0.8; transform: scale(1.1); }
}

.logo {
  width: 80px;
  height: 80px;
  color: #3b82f6;
  position: relative;
}

.hero-text h1 {
  font-size: 32px;
  font-weight: 600;
  color: #ffffff;
  margin: 0 0 8px 0;
  letter-spacing: -0.5px;
}

.hero-text p {
  font-size: 16px;
  color: rgba(255, 255, 255, 0.6);
  margin: 0;
}

.user-section {
  position: absolute;
  top: 16px;
  right: 16px;
}

.user-profile {
  display: flex;
  align-items: center;
  gap: 10px;
  padding: 6px 8px 6px 6px;
  background: rgba(255, 255, 255, 0.08);
  border: 1px solid rgba(255, 255, 255, 0.1);
  border-radius: 24px;
  backdrop-filter: blur(8px);
}

.user-avatar {
  width: 28px;
  height: 28px;
  border-radius: 50%;
  object-fit: cover;
  border: 2px solid rgba(255, 255, 255, 0.2);
}

.user-avatar-placeholder {
  width: 28px;
  height: 28px;
  border-radius: 50%;
  background: rgba(59, 130, 246, 0.3);
  display: flex;
  align-items: center;
  justify-content: center;
  border: 2px solid rgba(255, 255, 255, 0.2);
}

.avatar-icon {
  width: 16px;
  height: 16px;
  color: rgba(255, 255, 255, 0.7);
}

.user-name {
  font-size: 13px;
  font-weight: 500;
  color: rgba(255, 255, 255, 0.9);
  max-width: 120px;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.logout-btn {
  width: 26px;
  height: 26px;
  border-radius: 50%;
  border: none;
  background: transparent;
  color: rgba(255, 255, 255, 0.5);
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  transition: all 0.15s ease;
}

.logout-btn:hover {
  background: rgba(239, 68, 68, 0.2);
  color: #ef4444;
}

.logout-icon {
  width: 14px;
  height: 14px;
}

.login-btn {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 8px 16px;
  background: rgba(59, 130, 246, 0.2);
  border: 1px solid rgba(59, 130, 246, 0.3);
  border-radius: 20px;
  color: #3b82f6;
  font-size: 13px;
  font-weight: 500;
  cursor: pointer;
  transition: all 0.2s ease;
  backdrop-filter: blur(8px);
}

.login-btn:hover {
  background: rgba(59, 130, 246, 0.3);
  border-color: rgba(59, 130, 246, 0.5);
  transform: translateY(-1px);
}

.login-icon {
  width: 16px;
  height: 16px;
}

.actions-section {
  margin-bottom: 40px;
}

.action-cards {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 16px;
}

.action-card {
  display: flex;
  align-items: center;
  gap: 16px;
  padding: 20px 24px;
  background: rgba(255, 255, 255, 0.03);
  border: 1px solid rgba(255, 255, 255, 0.06);
  border-radius: 16px;
  cursor: pointer;
  transition: all 0.2s ease;
  text-align: left;
}

.action-card:hover {
  background: rgba(255, 255, 255, 0.06);
  border-color: rgba(255, 255, 255, 0.1);
  transform: translateY(-2px);
}

.action-card:active {
  transform: translateY(0);
}

.action-card.new-project:hover {
  border-color: rgba(59, 130, 246, 0.3);
  box-shadow: 0 8px 32px rgba(59, 130, 246, 0.15);
}

.action-card.open-project:hover {
  border-color: rgba(34, 197, 94, 0.3);
  box-shadow: 0 8px 32px rgba(34, 197, 94, 0.15);
}

.card-icon {
  width: 48px;
  height: 48px;
  border-radius: 12px;
  display: flex;
  align-items: center;
  justify-content: center;
  flex-shrink: 0;
}

.new-project .card-icon {
  background: linear-gradient(135deg, rgba(59, 130, 246, 0.2) 0%, rgba(59, 130, 246, 0.1) 100%);
}

.new-project .card-icon .icon {
  color: #3b82f6;
}

.open-project .card-icon {
  background: linear-gradient(135deg, rgba(34, 197, 94, 0.2) 0%, rgba(34, 197, 94, 0.1) 100%);
}

.open-project .card-icon .icon {
  color: #22c55e;
}

.card-icon .icon {
  width: 24px;
  height: 24px;
}

.card-content {
  flex: 1;
  min-width: 0;
}

.card-title {
  display: block;
  font-size: 15px;
  font-weight: 500;
  color: #ffffff;
  margin-bottom: 4px;
}

.card-subtitle {
  display: block;
  font-size: 13px;
  color: rgba(255, 255, 255, 0.5);
}

.card-arrow {
  width: 20px;
  height: 20px;
  color: rgba(255, 255, 255, 0.3);
  flex-shrink: 0;
  transition: transform 0.2s ease, color 0.2s ease;
}

.action-card:hover .card-arrow {
  color: rgba(255, 255, 255, 0.6);
  transform: translateX(4px);
}

.recent-section {
  animation: fadeIn 0.3s ease;
}

@keyframes fadeIn {
  from { opacity: 0; transform: translateY(10px); }
  to { opacity: 1; transform: translateY(0); }
}

.section-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin-bottom: 20px;
}

.header-left {
  display: flex;
  align-items: center;
  gap: 10px;
}

.header-icon {
  width: 18px;
  height: 18px;
  color: rgba(255, 255, 255, 0.4);
}

.section-header h2 {
  font-size: 16px;
  font-weight: 500;
  color: rgba(255, 255, 255, 0.9);
  margin: 0;
}

.project-count {
  font-size: 12px;
  font-weight: 500;
  color: rgba(255, 255, 255, 0.5);
  background: rgba(255, 255, 255, 0.1);
  padding: 2px 8px;
  border-radius: 10px;
}

.clear-btn {
  display: flex;
  align-items: center;
  gap: 6px;
  padding: 8px 12px;
  font-size: 13px;
  color: rgba(255, 255, 255, 0.5);
  background: transparent;
  border: 1px solid rgba(255, 255, 255, 0.1);
  border-radius: 8px;
  cursor: pointer;
  transition: all 0.2s ease;
}

.clear-btn:hover {
  color: #ef4444;
  border-color: rgba(239, 68, 68, 0.3);
  background: rgba(239, 68, 68, 0.1);
}

.btn-icon {
  width: 14px;
  height: 14px;
}

.loading-state {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  padding: 64px 0;
  gap: 16px;
  color: rgba(255, 255, 255, 0.5);
}

.loading-spinner {
  width: 32px;
  height: 32px;
  border: 3px solid rgba(255, 255, 255, 0.1);
  border-top-color: #3b82f6;
  border-radius: 50%;
  animation: spin 0.8s linear infinite;
}

@keyframes spin {
  to { transform: rotate(360deg); }
}

.empty-state {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  padding: 64px 0;
  text-align: center;
}

.empty-icon {
  width: 64px;
  height: 64px;
  border-radius: 16px;
  background: rgba(255, 255, 255, 0.05);
  display: flex;
  align-items: center;
  justify-content: center;
  margin-bottom: 16px;
}

.empty-icon .icon {
  width: 28px;
  height: 28px;
  color: rgba(255, 255, 255, 0.3);
}

.empty-state h3 {
  font-size: 16px;
  font-weight: 500;
  color: rgba(255, 255, 255, 0.7);
  margin: 0 0 8px 0;
}

.empty-state p {
  font-size: 14px;
  color: rgba(255, 255, 255, 0.4);
  margin: 0;
}

.recent-grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(200px, 1fr));
  gap: 16px;
}

@media (max-width: 768px) {
  .home-content {
    padding: 24px 20px 32px;
  }

  .hero-content {
    flex-direction: column;
    text-align: center;
    gap: 20px;
  }

  .hero-text h1 {
    font-size: 24px;
  }

  .action-cards {
    grid-template-columns: 1fr;
  }

  .recent-grid {
    grid-template-columns: repeat(auto-fill, minmax(160px, 1fr));
  }
}
</style>
