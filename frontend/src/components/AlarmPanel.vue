<script setup lang="ts">
import type { AlarmItem } from "../types";

defineProps<{
  alarms: AlarmItem[];
}>();
</script>

<template>
  <section class="card">
    <div class="card-header">
      <h2>Alarm Panel</h2>
      <span>Active Alerts</span>
    </div>
    <div class="alarm-list">
      <div v-for="alarm in alarms" :key="alarm.id" class="alarm-item">
        <div>
          <strong>{{ alarm.code }}</strong>
          <p>{{ alarm.message }}</p>
        </div>
        <span :class="['alarm-pill', alarm.level]">{{ alarm.level }}</span>
      </div>
    </div>
  </section>
</template>

