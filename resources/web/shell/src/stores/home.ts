import { defineStore } from 'pinia'
import { ref, computed } from 'vue'
import { useIPC } from '../composables/useIPC'

export interface RecentFile {
  path: string
  projectName: string
  time: string
  image?: string
}

export interface UserInfo {
  isLoggedIn: boolean
  userName: string
  userAvatar: string
  userId: string
}

export const useHomeStore = defineStore('home', () => {
  const { send } = useIPC()

  const recentFiles = ref<RecentFile[]>([])
  const isLoading = ref(false)
  const user = ref<UserInfo>({
    isLoggedIn: false,
    userName: '',
    userAvatar: '',
    userId: ''
  })

  const hasRecentFiles = computed(() => recentFiles.value.length > 0)
  const isLoggedIn = computed(() => user.value.isLoggedIn)

  const fetchRecentFiles = async () => {
    isLoading.value = true
    try {
      const response = await send('getRecentFiles')
      if (response.success && response.files) {
        recentFiles.value = response.files
      }
    } finally {
      isLoading.value = false
    }
  }

  const fetchUserInfo = async () => {
    const response = await send('getUserInfo')
    if (response.success) {
      user.value = {
        isLoggedIn: response.isLoggedIn,
        userName: response.userName || '',
        userAvatar: response.userAvatar || '',
        userId: response.userId || ''
      }
    }
  }

  const login = async () => {
    await send('userLogin')
  }

  const logout = async () => {
    await send('userLogout')
    user.value = {
      isLoggedIn: false,
      userName: '',
      userAvatar: '',
      userId: ''
    }
  }

  const openRecentFile = async (path: string) => {
    await send('openRecentFile', { path })
  }

  const deleteRecentFile = async (path: string) => {
    await send('deleteRecentFile', { path })
    recentFiles.value = recentFiles.value.filter(f => f.path !== path)
  }

  const clearAllRecentFiles = async () => {
    await send('clearAllRecentFiles')
    recentFiles.value = []
  }

  const exploreRecentFile = async (path: string) => {
    await send('exploreRecentFile', { path })
  }

  const newProject = async () => {
    await send('newProject')
  }

  const openProject = async () => {
    await send('openProject')
  }

  return {
    recentFiles,
    isLoading,
    hasRecentFiles,
    user,
    isLoggedIn,
    fetchRecentFiles,
    fetchUserInfo,
    openRecentFile,
    deleteRecentFile,
    clearAllRecentFiles,
    exploreRecentFile,
    newProject,
    openProject,
    login,
    logout,
  }
})
