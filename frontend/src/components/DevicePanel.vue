<script setup lang="ts">
import type { DeviceSummary } from "../types";

defineProps<{
  devices: DeviceSummary[];
  activeDeviceId: string;
}>();

const emit = defineEmits<{
  select: [deviceId: string];
}>();
</script>

<template>
  <section class="card">
    <div class="card-header">
      <h2>设备列表</h2>
      <span>Host Session Overview</span>
    </div>
    <div class="device-list">
      <button
        v-for="device in devices"
        :key="device.id"
        class="device-item"
        :class="{ active: device.id === activeDeviceId }"
        @click="emit('select', device.id)"
      >
        <div>
          <strong>{{ device.name }}</strong>
          <p>{{ device.mode.toUpperCase() }} · {{ device.ip }}:{{ device.port }}</p>
        </div>
        <span :class="['status-pill', device.status]">
          {{ device.selected ? "Selected" : device.status }}
        </span>
      </button>
    </div>
  </section>
</template>
