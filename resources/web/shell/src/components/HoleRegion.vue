<template>
  <div
    ref="holeElement"
    class="hole-region"
    :data-hole-id="holeId"
  />
</template>

<script setup lang="ts">
import { ref } from 'vue'
import { useHoleRegion } from '../composables/useLayout'

const props = defineProps<{
  holeId: string
}>()

const holeElement = ref<HTMLElement | null>(null)

useHoleRegion(props.holeId, holeElement)
</script>

<style scoped>
.hole-region {
  background: transparent;
  pointer-events: none;
}
</style>
