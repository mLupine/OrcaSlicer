<template>
  <div class="relative" ref="menuRef">
    <button
      class="flex items-center gap-1 px-3 py-1.5 text-sm text-navbar-text hover:bg-navbar-hover rounded transition-colors duration-150 focus:outline-none focus:ring-1 focus:ring-navbar-active"
      :aria-expanded="isOpen"
      aria-haspopup="true"
      @click="toggleMenu"
    >
      <Menu :size="16" :stroke-width="2" />
      <span class="font-medium">File</span>
    </button>

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
        class="absolute top-full left-0 mt-1 w-56 bg-navbar-bg border border-navbar-border rounded shadow-xl z-50"
        role="menu"
        @keydown.escape="closeMenu"
      >
        <div class="py-1">
          <button
            v-for="(item, index) in menuItems"
            :key="item.id"
            :ref="el => { if (el) itemRefs[index] = el as HTMLButtonElement }"
            class="w-full px-4 py-2 text-left text-sm text-navbar-text hover:bg-navbar-hover transition-colors duration-150 flex items-center justify-between focus:outline-none focus:bg-navbar-hover"
            :class="{ 'border-t border-navbar-border': item.separator }"
            role="menuitem"
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
import { ref, onMounted, onUnmounted, nextTick } from 'vue'
import { Menu } from 'lucide-vue-next'
import { useIPC } from '../composables/useIPC'

interface MenuItem {
  id: string
  label: string
  shortcut?: string
  separator?: boolean
}

const { send } = useIPC()
const isOpen = ref(false)
const menuRef = ref<HTMLElement | null>(null)
const itemRefs = ref<HTMLButtonElement[]>([])

const menuItems: MenuItem[] = [
  { id: 'new', label: 'New Project', shortcut: 'Ctrl+N' },
  { id: 'open', label: 'Open Project...', shortcut: 'Ctrl+O' },
  { id: 'save', label: 'Save', shortcut: 'Ctrl+S', separator: true },
  { id: 'save-as', label: 'Save As...' },
  { id: 'import', label: 'Import 3D File...', shortcut: 'Ctrl+I', separator: true },
  { id: 'export', label: 'Export G-code...', shortcut: 'Ctrl+G' },
  { id: 'recent', label: 'Recent Files', separator: true },
  { id: 'exit', label: 'Exit' },
]

const toggleMenu = () => {
  isOpen.value = !isOpen.value
  if (isOpen.value) {
    nextTick(() => {
      itemRefs.value[0]?.focus()
    })
  }
}

const closeMenu = () => {
  isOpen.value = false
}

const handleMenuClick = (itemId: string) => {
  send('file_menu_action', { action: itemId })
  closeMenu()
}

const handleClickOutside = (event: MouseEvent) => {
  if (menuRef.value && !menuRef.value.contains(event.target as Node)) {
    closeMenu()
  }
}

const focusNext = (currentIndex: number) => {
  const nextIndex = (currentIndex + 1) % itemRefs.value.length
  itemRefs.value[nextIndex]?.focus()
}

const focusPrevious = (currentIndex: number) => {
  const prevIndex = currentIndex === 0 ? itemRefs.value.length - 1 : currentIndex - 1
  itemRefs.value[prevIndex]?.focus()
}

onMounted(() => {
  document.addEventListener('click', handleClickOutside)
})

onUnmounted(() => {
  document.removeEventListener('click', handleClickOutside)
})
</script>
