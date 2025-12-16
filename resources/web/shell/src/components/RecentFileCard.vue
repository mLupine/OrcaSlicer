<template>
  <div
    class="file-card"
    @click="$emit('open')"
    @contextmenu.prevent="toggleMenu"
  >
    <div class="thumbnail-container">
      <div class="thumbnail">
        <img
          v-if="file.image && !imageError"
          :src="file.image"
          :alt="file.projectName"
          @error="imageError = true"
        />
        <div v-else class="placeholder">
          <Box class="placeholder-icon" />
        </div>
      </div>
      <div class="overlay">
        <div class="overlay-actions">
          <button class="action-btn open-btn" @click.stop="$emit('open')" title="Open">
            <ExternalLink class="btn-icon" />
          </button>
          <button class="action-btn menu-btn" @click.stop="toggleMenu" title="More">
            <MoreVertical class="btn-icon" />
          </button>
        </div>
      </div>
    </div>

    <div class="file-info">
      <span class="file-name" :title="file.path">{{ file.projectName }}</span>
      <span class="file-time">{{ formatTime(file.time) }}</span>
    </div>

    <Teleport to="body">
      <Transition name="menu">
        <div
          v-if="showMenu"
          class="context-menu"
          :style="menuPosition"
          @click.stop
        >
          <div class="menu-backdrop" @click="showMenu = false" />
          <div class="menu-content">
            <button class="menu-item" @click="handleOpen">
              <ExternalLink class="menu-icon" />
              <span>Open Project</span>
            </button>
            <button class="menu-item" @click="handleExplore">
              <Folder class="menu-icon" />
              <span>Show in Finder</span>
            </button>
            <div class="menu-divider" />
            <button class="menu-item danger" @click="handleDelete">
              <Trash2 class="menu-icon" />
              <span>Remove from List</span>
            </button>
          </div>
        </div>
      </Transition>
    </Teleport>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, onMounted, onUnmounted } from 'vue'
import { Box, ExternalLink, MoreVertical, Folder, Trash2 } from 'lucide-vue-next'
import type { RecentFile } from '../stores/home'

defineProps<{
  file: RecentFile
}>()

const emit = defineEmits<{
  open: []
  delete: []
  explore: []
}>()

const showMenu = ref(false)
const menuX = ref(0)
const menuY = ref(0)
const imageError = ref(false)

const menuPosition = computed(() => ({
  left: `${menuX.value}px`,
  top: `${menuY.value}px`,
}))

const formatTime = (time: string): string => {
  if (!time) return ''

  try {
    const date = new Date(time.replace(' ', 'T'))
    const now = new Date()
    const diff = now.getTime() - date.getTime()
    const days = Math.floor(diff / (1000 * 60 * 60 * 24))

    if (days === 0) {
      return 'Today'
    } else if (days === 1) {
      return 'Yesterday'
    } else if (days < 7) {
      return `${days} days ago`
    } else if (days < 30) {
      const weeks = Math.floor(days / 7)
      return `${weeks} week${weeks > 1 ? 's' : ''} ago`
    } else {
      return date.toLocaleDateString('en-US', {
        month: 'short',
        day: 'numeric',
        year: date.getFullYear() !== now.getFullYear() ? 'numeric' : undefined
      })
    }
  } catch {
    return time
  }
}

const toggleMenu = (e: MouseEvent) => {
  if (showMenu.value) {
    showMenu.value = false
    return
  }

  menuX.value = Math.min(e.clientX, window.innerWidth - 200)
  menuY.value = Math.min(e.clientY, window.innerHeight - 160)
  showMenu.value = true
}

const handleOpen = () => {
  showMenu.value = false
  emit('open')
}

const handleDelete = () => {
  showMenu.value = false
  emit('delete')
}

const handleExplore = () => {
  showMenu.value = false
  emit('explore')
}

const handleKeydown = (e: KeyboardEvent) => {
  if (e.key === 'Escape') {
    showMenu.value = false
  }
}

onMounted(() => {
  document.addEventListener('keydown', handleKeydown)
})

onUnmounted(() => {
  document.removeEventListener('keydown', handleKeydown)
})
</script>

<style scoped>
.file-card {
  position: relative;
  border-radius: 12px;
  overflow: hidden;
  background: rgba(255, 255, 255, 0.03);
  border: 1px solid rgba(255, 255, 255, 0.06);
  cursor: pointer;
  transition: all 0.2s ease;
  animation: cardAppear 0.3s ease backwards;
  animation-delay: var(--delay, 0ms);
}

@keyframes cardAppear {
  from {
    opacity: 0;
    transform: translateY(10px) scale(0.98);
  }
  to {
    opacity: 1;
    transform: translateY(0) scale(1);
  }
}

.file-card:hover {
  background: rgba(255, 255, 255, 0.06);
  border-color: rgba(255, 255, 255, 0.12);
  transform: translateY(-4px);
  box-shadow: 0 12px 40px rgba(0, 0, 0, 0.3);
}

.thumbnail-container {
  position: relative;
  aspect-ratio: 4 / 3;
  overflow: hidden;
}

.thumbnail {
  width: 100%;
  height: 100%;
  background: linear-gradient(135deg, #1f1f23 0%, #17171a 100%);
}

.thumbnail img {
  width: 100%;
  height: 100%;
  object-fit: cover;
  transition: transform 0.3s ease;
}

.file-card:hover .thumbnail img {
  transform: scale(1.05);
}

.placeholder {
  width: 100%;
  height: 100%;
  display: flex;
  align-items: center;
  justify-content: center;
  background: linear-gradient(135deg, rgba(59, 130, 246, 0.1) 0%, rgba(147, 51, 234, 0.05) 100%);
}

.placeholder-icon {
  width: 40px;
  height: 40px;
  color: rgba(255, 255, 255, 0.2);
}

.overlay {
  position: absolute;
  inset: 0;
  background: linear-gradient(180deg, transparent 40%, rgba(0, 0, 0, 0.6) 100%);
  opacity: 0;
  transition: opacity 0.2s ease;
  display: flex;
  align-items: flex-end;
  justify-content: flex-end;
  padding: 8px;
}

.file-card:hover .overlay {
  opacity: 1;
}

.overlay-actions {
  display: flex;
  gap: 4px;
}

.action-btn {
  width: 32px;
  height: 32px;
  border-radius: 8px;
  border: none;
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  transition: all 0.15s ease;
}

.open-btn {
  background: rgba(59, 130, 246, 0.9);
  color: white;
}

.open-btn:hover {
  background: #3b82f6;
  transform: scale(1.05);
}

.menu-btn {
  background: rgba(255, 255, 255, 0.15);
  color: white;
  backdrop-filter: blur(8px);
}

.menu-btn:hover {
  background: rgba(255, 255, 255, 0.25);
}

.btn-icon {
  width: 16px;
  height: 16px;
}

.file-info {
  padding: 12px;
}

.file-name {
  display: block;
  font-size: 13px;
  font-weight: 500;
  color: rgba(255, 255, 255, 0.9);
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
  margin-bottom: 4px;
}

.file-time {
  font-size: 11px;
  color: rgba(255, 255, 255, 0.4);
}

.context-menu {
  position: fixed;
  z-index: 10000;
}

.menu-backdrop {
  position: fixed;
  inset: 0;
}

.menu-content {
  position: relative;
  min-width: 180px;
  background: #2a2a2e;
  border: 1px solid rgba(255, 255, 255, 0.1);
  border-radius: 12px;
  padding: 6px;
  box-shadow: 0 16px 48px rgba(0, 0, 0, 0.4);
}

.menu-item {
  display: flex;
  align-items: center;
  gap: 10px;
  width: 100%;
  padding: 10px 12px;
  font-size: 13px;
  color: rgba(255, 255, 255, 0.85);
  background: transparent;
  border: none;
  border-radius: 8px;
  cursor: pointer;
  transition: all 0.15s ease;
  text-align: left;
}

.menu-item:hover {
  background: rgba(255, 255, 255, 0.08);
}

.menu-item.danger {
  color: #ef4444;
}

.menu-item.danger:hover {
  background: rgba(239, 68, 68, 0.15);
}

.menu-icon {
  width: 16px;
  height: 16px;
  opacity: 0.7;
}

.menu-divider {
  height: 1px;
  background: rgba(255, 255, 255, 0.08);
  margin: 6px 0;
}

.menu-enter-active,
.menu-leave-active {
  transition: all 0.15s ease;
}

.menu-enter-from,
.menu-leave-to {
  opacity: 0;
  transform: scale(0.95);
}

.menu-enter-from .menu-content,
.menu-leave-to .menu-content {
  transform: translateY(-4px);
}
</style>
