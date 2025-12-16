import { defineStore } from 'pinia'
import { ref, computed } from 'vue'
import type { HoleBounds } from '../types/layout'
import { useIPC } from '../composables/useIPC'

export const useLayoutStore = defineStore('layout', () => {
  const { send } = useIPC()

  const sidebarWidth = ref(300)
  const holes = ref<Record<string, HoleBounds>>({})

  const showSidebar = computed(() => {
    return true
  })

  const registerHole = (id: string) => {
    if (!holes.value[id]) {
      holes.value[id] = { x: 0, y: 0, width: 0, height: 0 }
    }
  }

  const unregisterHole = (id: string) => {
    delete holes.value[id]
  }

  const updateHoleBounds = async (id: string, bounds: HoleBounds) => {
    holes.value[id] = bounds

    try {
      await send('updateHoleBounds', { id, bounds })
    } catch (error) {
      console.error('Failed to update hole bounds:', error)
    }
  }

  const getHoleBounds = (id: string): HoleBounds | undefined => {
    return holes.value[id]
  }

  const setSidebarWidth = (width: number) => {
    sidebarWidth.value = width
  }

  return {
    sidebarWidth,
    holes,
    showSidebar,
    registerHole,
    unregisterHole,
    updateHoleBounds,
    getHoleBounds,
    setSidebarWidth,
  }
})
