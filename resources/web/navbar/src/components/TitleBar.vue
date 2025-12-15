<template>
  <div
    class="flex items-center justify-center h-full px-4 select-none"
    :class="{ 'cursor-move': isDraggable }"
  >
    <span
      class="text-sm font-medium text-navbar-text truncate max-w-md"
      :title="fullTitle"
    >
      {{ displayTitle }}
    </span>
  </div>
</template>

<script setup lang="ts">
import { computed } from 'vue'
import { storeToRefs } from 'pinia'
import { useAppStore } from '../stores/app'

const appStore = useAppStore()
const { title } = storeToRefs(appStore)

const isDraggable = computed(() => {
  return !navigator.platform.toLowerCase().includes('mac')
})

const fullTitle = computed(() => {
  return title.value
})

const displayTitle = computed(() => {
  const maxLength = 50
  if (title.value.length <= maxLength) {
    return title.value
  }
  return title.value.substring(0, maxLength - 3) + '...'
})
</script>
