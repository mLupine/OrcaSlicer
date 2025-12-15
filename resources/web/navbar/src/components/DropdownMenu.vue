<template>
  <div class="relative" ref="menuRef">
    <IconButton icon="more" @click="toggleMenu" :tooltip="'More options'" />

    <Transition
      enter-active-class="transition ease-out duration-100"
      enter-from-class="transform opacity-0 scale-95"
      enter-to-class="transform opacity-100 scale-100"
      leave-active-class="transition ease-in duration-75"
      leave-from-class="transform opacity-100 scale-100"
      leave-to-class="transform opacity-0 scale-95"
    >
      <div
        v-if="isOpen"
        class="absolute top-full right-0 mt-1 w-52 bg-navbar-bg border border-navbar-border rounded shadow-xl z-50"
        role="menu"
        @keydown.escape="closeMenu"
      >
        <div class="py-1">
          <button
            v-for="(item, index) in menuItems"
            :key="item.id"
            :ref="el => { if (el) itemRefs[index] = el as HTMLButtonElement }"
            class="w-full px-4 py-2 text-left text-sm transition-colors duration-150 flex items-center justify-between focus:outline-none focus:bg-navbar-hover"
            :class="[
              item.disabled
                ? 'text-navbar-text-secondary opacity-50 cursor-not-allowed'
                : 'text-navbar-text hover:bg-navbar-hover',
              { 'border-t border-navbar-border': item.separator }
            ]"
            role="menuitem"
            :disabled="item.disabled"
            :tabindex="isOpen ? 0 : -1"
            @click="handleMenuClick(item.id)"
            @keydown.down.prevent="focusNext(index)"
            @keydown.up.prevent="focusPrevious(index)"
          >
            <span>{{ item.label }}</span>
            <span v-if="item.shortcut" class="text-xs text-navbar-text-secondary ml-4">
              {{ item.shortcut }}
            </span>
          </button>
        </div>
      </div>
    </Transition>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, onMounted, onUnmounted, nextTick } from 'vue'
import { useIPC } from '../composables/useIPC'
import IconButton from './IconButton.vue'

interface MenuItem {
  id: string
  label: string
  disabled: boolean
  shortcut?: string
  separator?: boolean
}

const { send } = useIPC()

const isOpen = ref(false)
const menuRef = ref<HTMLElement | null>(null)
const itemRefs = ref<HTMLButtonElement[]>([])

const menuItems = computed<MenuItem[]>(() => [
  { id: 'preferences', label: 'Preferences...', disabled: false, shortcut: 'Ctrl+,' },
  { id: 'printer-settings', label: 'Printer Settings', disabled: false, separator: true },
  { id: 'filament-settings', label: 'Filament Settings', disabled: false },
  { id: 'publish', label: 'Publish Model', disabled: false, separator: true },
  { id: 'help', label: 'Help', disabled: false, shortcut: 'F1' },
  { id: 'about', label: 'About OrcaSlicer', disabled: false },
])

const toggleMenu = () => {
  isOpen.value = !isOpen.value
  if (isOpen.value) {
    nextTick(() => {
      const firstEnabled = itemRefs.value.find(item => !item.disabled)
      firstEnabled?.focus()
    })
  }
}

const closeMenu = () => {
  isOpen.value = false
}

const handleMenuClick = (itemId: string) => {
  const item = menuItems.value.find(i => i.id === itemId)
  if (item && !item.disabled) {
    send('menu_action', { action: itemId })
    closeMenu()
  }
}

const handleClickOutside = (event: MouseEvent) => {
  if (menuRef.value && !menuRef.value.contains(event.target as Node)) {
    closeMenu()
  }
}

const focusNext = (currentIndex: number) => {
  let nextIndex = (currentIndex + 1) % itemRefs.value.length
  while (itemRefs.value[nextIndex]?.disabled && nextIndex !== currentIndex) {
    nextIndex = (nextIndex + 1) % itemRefs.value.length
  }
  itemRefs.value[nextIndex]?.focus()
}

const focusPrevious = (currentIndex: number) => {
  let prevIndex = currentIndex === 0 ? itemRefs.value.length - 1 : currentIndex - 1
  while (itemRefs.value[prevIndex]?.disabled && prevIndex !== currentIndex) {
    prevIndex = prevIndex === 0 ? itemRefs.value.length - 1 : prevIndex - 1
  }
  itemRefs.value[prevIndex]?.focus()
}

onMounted(() => {
  document.addEventListener('click', handleClickOutside)
})

onUnmounted(() => {
  document.removeEventListener('click', handleClickOutside)
})
</script>
