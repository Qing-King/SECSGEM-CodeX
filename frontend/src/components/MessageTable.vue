<script setup lang="ts">
import type { MessageEvent } from "../types";

defineProps<{
  messages: MessageEvent[];
}>();

const emit = defineEmits<{
  inspect: [message: MessageEvent];
}>();
</script>

<template>
  <section class="card card-wide">
    <div class="card-header">
      <h2>Recent Messages</h2>
      <span>REST + WebSocket</span>
    </div>
    <el-table :data="messages" style="width: 100%" @row-click="emit('inspect', $event)">
      <el-table-column prop="timestamp" label="Time" width="120" />
      <el-table-column prop="direction" label="Direction" width="100" />
      <el-table-column label="Stream/Function" width="140">
        <template #default="{ row }">S{{ row.stream }}F{{ row.function }}</template>
      </el-table-column>
      <el-table-column prop="note" label="Note" />
    </el-table>
  </section>
</template>

