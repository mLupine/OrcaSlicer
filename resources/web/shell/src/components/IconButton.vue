<template>
  <button
    class="p-1.5 rounded transition-colors duration-150 focus:outline-none focus:ring-1 focus:ring-navbar-active"
    :class="buttonClasses"
    :disabled="disabled"
    :title="tooltip"
    :aria-label="tooltip || icon"
    @click="$emit('click')"
  >
    <component :is="iconComponent" :size="size" :stroke-width="2" />
  </button>
</template>

<script setup lang="ts">
import { computed } from 'vue'
import {
  Undo2,
  Redo2,
  Save,
  Menu,
  ChevronDown,
  MoreVertical
} from 'lucide-vue-next'

interface Props {
  icon: string
  disabled?: boolean
  tooltip?: string
  size?: number
}

const props = withDefaults(defineProps<Props>(), {
  disabled: false,
  tooltip: '',
  size: 18,
})

defineEmits<{ click: [] }>()

const iconMap: Record<string, any> = {
  undo: Undo2,
  redo: Redo2,
  save: Save,
  menu: Menu,
  chevronDown: ChevronDown,
  more: MoreVertical,
}

const iconComponent = computed(() => iconMap[props.icon] || MoreVertical)

const buttonClasses = computed(() => ({
  'text-navbar-text hover:bg-navbar-hover hover:text-white active:bg-navbar-active': !props.disabled,
  'text-navbar-text-secondary opacity-40 cursor-not-allowed': props.disabled,
}))
</script>
