<template>
  <div class="content-area flex-1 relative">
    <HomeView v-if="isHomeTab" class="absolute inset-0" />
    <HoleRegion v-else holeId="main-content" class="absolute inset-0" />
  </div>
</template>

<script setup lang="ts">
import { computed } from 'vue'
import { useNavigationStore } from '../stores/navigation'
import HoleRegion from './HoleRegion.vue'
import HomeView from '../views/HomeView.vue'

const navigationStore = useNavigationStore()

const isHomeTab = computed(() => navigationStore.activeTab === 'home')
</script>

<style scoped>
.content-area {
  background: transparent;
}
</style>
