<template>
  <div v-if="isMacOS" class="h-full w-[78px]"></div>
  <div v-else class="flex items-center h-full">
    <button
      class="px-4 h-full text-navbar-text hover:bg-navbar-hover transition-colors duration-150 focus:outline-none"
      :aria-label="'Minimize window'"
      @click="minimize"
    >
      <Minus :size="16" :stroke-width="2" />
    </button>
    <button
      class="px-4 h-full text-navbar-text hover:bg-navbar-hover transition-colors duration-150 focus:outline-none"
      :aria-label="isMaximized ? 'Restore window' : 'Maximize window'"
      @click="maximize"
    >
      <component :is="isMaximized ? Minimize2 : Maximize2" :size="16" :stroke-width="2" />
    </button>
    <button
      class="px-4 h-full text-navbar-text hover:bg-red-600 hover:text-white transition-colors duration-150 focus:outline-none"
      :aria-label="'Close window'"
      @click="close"
    >
      <X :size="16" :stroke-width="2" />
    </button>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue'
import { Minus, Maximize2, Minimize2, X } from 'lucide-vue-next'
import { useIPC } from '../composables/useIPC'

const { send } = useIPC()
const isMacOS = ref(false)
const isMaximized = ref(false)

onMounted(() => {
  isMacOS.value = navigator.platform.toLowerCase().includes('mac')

  window.addEventListener('resize', checkMaximized)
  checkMaximized()
})

const checkMaximized = () => {
  if (window.outerWidth === window.screen.width && window.outerHeight === window.screen.height) {
    isMaximized.value = true
  } else {
    isMaximized.value = false
  }
}

const minimize = () => {
  send('windowMinimize')
}

const maximize = () => {
  send('windowMaximize')
  setTimeout(checkMaximized, 100)
}

const close = () => {
  send('windowClose')
}
</script>
