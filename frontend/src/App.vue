<script setup lang="ts">
import { computed, onMounted, ref } from "vue";
import { ElMessage } from "element-plus";
import AlarmPanel from "./components/AlarmPanel.vue";
import ArchitectureCard from "./components/ArchitectureCard.vue";
import ConnectionToolbar from "./components/ConnectionToolbar.vue";
import DevicePanel from "./components/DevicePanel.vue";
import MessageDetailDrawer from "./components/MessageDetailDrawer.vue";
import MessageTable from "./components/MessageTable.vue";
import { alarmList, deviceList as defaultDevices, runtimeMetrics } from "./mock-data";
import { fetchDevices, fetchRecentMessages } from "./services/api";
import { MockWsClient } from "./services/ws";
import type { AlarmItem, DeviceSummary, MessageEvent } from "./types";

const devices = ref<DeviceSummary[]>(defaultDevices);
const activeDeviceId = ref("eqp01");
const messages = ref<MessageEvent[]>([]);
const alarms = ref<AlarmItem[]>(alarmList);
const drawerVisible = ref(false);
const selectedMessage = ref<MessageEvent | null>(null);

const activeDevice = computed(() =>
  devices.value.find((device) => device.id === activeDeviceId.value) ?? null
);

async function loadMessages(): Promise<void> {
  messages.value = await fetchRecentMessages(activeDeviceId.value);
}

function openMessageDetail(message: MessageEvent): void {
  selectedMessage.value = message;
  drawerVisible.value = true;
}

function closeDrawer(): void {
  drawerVisible.value = false;
}

function notifyAction(action: string): void {
  ElMessage({
    message: `${action} command queued for ${activeDevice.value?.name ?? "device"}.`,
    type: "success"
  });
}

onMounted(async () => {
  devices.value = await fetchDevices();
  await loadMessages();

  const ws = new MockWsClient((event) => {
    if (event.type !== "secs_message" || typeof event.payload !== "object" || event.payload === null) {
      return;
    }

    const payload = event.payload as Record<string, unknown>;
    messages.value = [
      {
        id: `ws-${Date.now()}`,
        timestamp: new Date().toLocaleTimeString("en-GB", { hour12: false }),
        direction: payload.direction === "send" ? "send" : "recv",
        stream: typeof payload.stream === "number" ? payload.stream : 0,
        function: typeof payload.function === "number" ? payload.function : 0,
        note: typeof payload.note === "string" ? payload.note : "Unknown"
      },
      ...messages.value
    ].slice(0, 12);
  });

  ws.connect();
});
</script>

<template>
  <div class="page-shell">
    <header class="hero-card">
      <div>
        <p class="eyebrow">SECS/GEM Host Console</p>
        <h1>Realtime Equipment Console</h1>
        <p class="hero-copy">
          This runtime page is separated from the design documents. It now includes connection controls,
          active alarms, and a message detail drawer on top of the earlier dashboard shell.
        </p>
      </div>
      <div class="hero-stack">
        <span>Vue 3</span>
        <span>Element Plus</span>
        <span>ECharts</span>
        <span>WebSocket</span>
      </div>
    </header>

    <section class="metrics-grid">
      <article v-for="item in runtimeMetrics" :key="item.label" class="metric-card">
        <p>{{ item.label }}</p>
        <strong>{{ item.value }}</strong>
      </article>
    </section>

    <ConnectionToolbar
      :active-device="activeDevice"
      @connect="notifyAction('Connect')"
      @disconnect="notifyAction('Disconnect')"
      @linktest="notifyAction('Linktest')"
      @refresh="loadMessages"
    />

    <main class="main-grid">
      <DevicePanel
        :devices="devices"
        :active-device-id="activeDeviceId"
        @select="activeDeviceId = $event; loadMessages()"
      />

      <section class="card">
        <div class="card-header">
          <h2>Device Snapshot</h2>
          <span>Selected Equipment</span>
        </div>
        <div v-if="activeDevice" class="detail-grid">
          <div class="detail-item">
            <span>Name</span>
            <strong>{{ activeDevice.name }}</strong>
          </div>
          <div class="detail-item">
            <span>Mode</span>
            <strong>{{ activeDevice.mode.toUpperCase() }}</strong>
          </div>
          <div class="detail-item">
            <span>Endpoint</span>
            <strong>{{ activeDevice.ip }}:{{ activeDevice.port }}</strong>
          </div>
          <div class="detail-item">
            <span>Session</span>
            <strong>{{ activeDevice.selected ? "Selected" : activeDevice.status }}</strong>
          </div>
        </div>
      </section>

      <AlarmPanel :alarms="alarms" />
      <ArchitectureCard />
      <MessageTable :messages="messages" @inspect="openMessageDetail" />
    </main>

    <MessageDetailDrawer
      :visible="drawerVisible"
      :message="selectedMessage"
      @close="closeDrawer"
    />
  </div>
</template>

