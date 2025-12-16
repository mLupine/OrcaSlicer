<template>
  <div class="shell flex flex-col w-full h-full">
    <UnsavedIndicator />
    <header class="navbar-container shrink-0">
      <Navbar />
    </header>
    <ContentArea class="flex-1" />
  </div>
</template>

<script setup lang="ts">
import Navbar from './Navbar.vue'
import UnsavedIndicator from './UnsavedIndicator.vue'
import ContentArea from './ContentArea.vue'
</script>

<style scoped>
.shell {
  background: transparent;
}

.navbar-container {
  background-color: #222224;
}
</style>
