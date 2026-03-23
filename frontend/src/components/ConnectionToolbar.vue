<script setup lang="ts">
import type { DeviceSummary } from "../types";

defineProps<{
  activeDevice: DeviceSummary | null;
}>();

const emit = defineEmits<{
  connect: [];
  disconnect: [];
  refresh: [];
  linktest: [];
}>();
</script>

<template>
  <section class="card card-wide">
    <div class="card-header">
      <h2>Connection Toolbar</h2>
      <span>Host Controls</span>
    </div>
    <div class="toolbar-row">
      <div class="toolbar-meta" v-if="activeDevice">
        <strong>{{ activeDevice.name }}</strong>
        <span>{{ activeDevice.ip }}:{{ activeDevice.port }}</span>
      </div>
      <div class="toolbar-actions">
        <el-button type="primary" @click="emit('connect')">Connect</el-button>
        <el-button @click="emit('disconnect')">Disconnect</el-button>
        <el-button @click="emit('linktest')">Linktest</el-button>
        <el-button @click="emit('refresh')">Refresh</el-button>
      </div>
    </div>
  </section>
</template>

