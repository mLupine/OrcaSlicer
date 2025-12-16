<template>
  <div class="relative w-full h-full bg-navbar-bg">
    <UnsavedIndicator />
    <Navbar />
  </div>
</template>

<script setup lang="ts">
import Navbar from './components/Navbar.vue'
import UnsavedIndicator from './components/UnsavedIndicator.vue'
</script>
