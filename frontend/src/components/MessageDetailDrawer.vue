<script setup lang="ts">
import type { MessageEvent } from "../types";

defineProps<{
  visible: boolean;
  message: MessageEvent | null;
}>();

const emit = defineEmits<{
  close: [];
}>();
</script>

<template>
  <el-drawer :model-value="visible" title="Message Detail" size="36%" @close="emit('close')">
    <div v-if="message" class="drawer-stack">
      <div class="detail-item">
        <span>Timestamp</span>
        <strong>{{ message.timestamp }}</strong>
      </div>
      <div class="detail-item">
        <span>Direction</span>
        <strong>{{ message.direction }}</strong>
      </div>
      <div class="detail-item">
        <span>Stream / Function</span>
        <strong>S{{ message.stream }}F{{ message.function }}</strong>
      </div>
      <div class="detail-item">
        <span>Note</span>
        <strong>{{ message.note }}</strong>
      </div>
    </div>
  </el-drawer>
</template>

