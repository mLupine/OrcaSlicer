<template>
  <div
    class="unsaved-indicator"
    :class="{
      'unsaved-indicator--visible': isVisible,
      'unsaved-indicator--saving': isSaving,
    }"
  />
</template>

<script setup lang="ts">
import { ref, watch, onUnmounted } from 'vue'
import { storeToRefs } from 'pinia'
import { useAppStore } from '../stores/app'

const appStore = useAppStore()
const { hasUnsavedChanges, justSaved } = storeToRefs(appStore)

const isVisible = ref(false)
const isSaving = ref(false)

let hideTimeout: ReturnType<typeof setTimeout> | null = null

const clearHideTimeout = () => {
  if (hideTimeout) {
    clearTimeout(hideTimeout)
    hideTimeout = null
  }
}

watch(hasUnsavedChanges, (newValue) => {
  clearHideTimeout()
  if (newValue) {
    isVisible.value = true
    isSaving.value = false
  }
}, { immediate: true })

watch(justSaved, (newValue) => {
  if (newValue) {
    isSaving.value = true
    hideTimeout = setTimeout(() => {
      isVisible.value = false
      isSaving.value = false
      appStore.clearJustSaved()
    }, 1500)
  }
})

onUnmounted(() => {
  clearHideTimeout()
})
</script>

<style scoped>
.unsaved-indicator {
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  height: 3px;
  background: linear-gradient(90deg, #3b82f6, #60a5fa, #3b82f6);
  background-size: 200% 100%;
  opacity: 0;
  transform: scaleY(0);
  transform-origin: top;
  transition: opacity 0.3s ease, transform 0.3s ease, background 0.4s ease;
  z-index: 9999;
  pointer-events: none;
}

.unsaved-indicator--visible {
  opacity: 1;
  transform: scaleY(1);
  animation: shimmer 2s ease-in-out infinite;
}

.unsaved-indicator--saving {
  background: linear-gradient(90deg, #22c55e, #4ade80, #22c55e);
  animation: none;
}

@keyframes shimmer {
  0%, 100% {
    background-position: 0% 50%;
  }
  50% {
    background-position: 100% 50%;
  }
}
</style>
