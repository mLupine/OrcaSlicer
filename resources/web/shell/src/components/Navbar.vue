<template>
  <div class="flex items-center h-10 bg-navbar-bg pl-[78px]">
    <div class="flex items-center gap-1 px-2">
      <IconButton icon="undo" :disabled="!canUndo" @click="handleUndo" />
      <IconButton icon="redo" :disabled="!canRedo" @click="handleRedo" />
    </div>

    <div class="flex items-center gap-0.5 px-2">
      <TabButton
        v-for="tab in visibleTabs"
        :key="tab.id"
        :tab="tab"
        :active="activeTab === tab.id"
        @click="handleTabClick(tab.id)"
      />
    </div>

    <TitleBar class="flex-1" />

    <WindowControls />
  </div>
</template>

<script setup lang="ts">
import { storeToRefs } from 'pinia'
import { useNavigationStore } from '../stores/navigation'
import { useAppStore } from '../stores/app'
import { useNavigation } from '../composables/useNavigation'
import { useIPC } from '../composables/useIPC'
import TabButton from './TabButton.vue'
import IconButton from './IconButton.vue'
import TitleBar from './TitleBar.vue'
import WindowControls from './WindowControls.vue'

const navigationStore = useNavigationStore()
const appStore = useAppStore()
const { selectTab } = useNavigation()
const { send } = useIPC()

const { activeTab, visibleTabs } = storeToRefs(navigationStore)
const { canUndo, canRedo } = storeToRefs(appStore)

const handleTabClick = (tabId: string) => {
  selectTab(tabId)
}

const handleUndo = () => {
  send('undo')
}

const handleRedo = () => {
  send('redo')
}
</script>
