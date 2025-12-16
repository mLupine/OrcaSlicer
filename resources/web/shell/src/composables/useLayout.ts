import { ref, onMounted, onUnmounted, type Ref } from 'vue'
import { useLayoutStore } from '../stores/layout'
import type { HoleBounds } from '../types/layout'

export function useHoleRegion(holeId: string, elementRef: Ref<HTMLElement | null>) {
  const layoutStore = useLayoutStore()
  const resizeObserver = ref<ResizeObserver | null>(null)

  const updateBounds = () => {
    if (!elementRef.value) return

    const rect = elementRef.value.getBoundingClientRect()
    const bounds: HoleBounds = {
      x: Math.round(rect.left),
      y: Math.round(rect.top),
      width: Math.round(rect.width),
      height: Math.round(rect.height),
    }

    layoutStore.updateHoleBounds(holeId, bounds)
  }

  onMounted(() => {
    layoutStore.registerHole(holeId)

    if (elementRef.value) {
      resizeObserver.value = new ResizeObserver(() => {
        updateBounds()
      })
      resizeObserver.value.observe(elementRef.value)

      updateBounds()
    }

    window.addEventListener('resize', updateBounds)
  })

  onUnmounted(() => {
    layoutStore.unregisterHole(holeId)

    if (resizeObserver.value) {
      resizeObserver.value.disconnect()
    }

    window.removeEventListener('resize', updateBounds)
  })

  return {
    updateBounds,
  }
}
