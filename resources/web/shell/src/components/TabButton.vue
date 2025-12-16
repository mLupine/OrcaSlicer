<template>
  <button
    class="flex items-center gap-1.5 px-3 py-1 rounded transition-colors duration-150 focus:outline-none focus:ring-1 focus:ring-navbar-active"
    :class="buttonClasses"
    :aria-label="tab.label"
    :aria-current="active ? 'page' : undefined"
    @click="$emit('click')"
  >
    <component :is="iconComponent" :size="16" :stroke-width="2" />
    <span v-if="tab.label !== 'Home'" class="text-sm font-medium">{{ tab.label }}</span>
  </button>
</template>

<script setup lang="ts">
import { computed } from 'vue'
import {
  Home,
  Box,
  Eye,
  Printer,
  LayoutGrid,
  FolderOpen,
  Settings2
} from 'lucide-vue-next'
import type { Tab } from '../stores/navigation'

interface Props {
  tab: Tab
  active: boolean
}

const props = defineProps<Props>()
defineEmits<{ click: [] }>()

const iconMap: Record<string, any> = {
  home: Home,
  box: Box,
  eye: Eye,
  printer: Printer,
  layoutGrid: LayoutGrid,
  folderOpen: FolderOpen,
  settings2: Settings2,
}

const iconComponent = computed(() => iconMap[props.tab.icon] || Box)

const buttonClasses = computed(() => ({
  'bg-navbar-active text-white': props.active,
  'text-navbar-text-secondary hover:bg-navbar-hover hover:text-navbar-text': !props.active,
}))
</script>
