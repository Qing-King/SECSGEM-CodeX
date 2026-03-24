<script setup lang="ts">
import { computed, onBeforeUnmount, onMounted, ref, watch } from "vue";
import { ElMessage } from "element-plus";
import AlarmPanel from "./components/AlarmPanel.vue";
import ArchitectureCard from "./components/ArchitectureCard.vue";
import ConnectionToolbar from "./components/ConnectionToolbar.vue";
import DevicePanel from "./components/DevicePanel.vue";
import MessageDetailDrawer from "./components/MessageDetailDrawer.vue";
import MessageTable from "./components/MessageTable.vue";
import { alarmList, deviceList as defaultDevices, runtimeMetrics } from "./mock-data";
import {
  connectDevice,
  disconnectDevice,
  fetchDevices,
  fetchRecentMessages,
  linktestDevice
} from "./services/api";
import { DeviceWsClient } from "./services/ws";
import type { AlarmItem, DeviceSummary, MessageEvent, WsDeviceEvent } from "./types";

const devices = ref<DeviceSummary[]>(defaultDevices);
const activeDeviceId = ref("eqp01");
const messages = ref<MessageEvent[]>([]);
const alarms = ref<AlarmItem[]>(alarmList);
const drawerVisible = ref(false);
const selectedMessage = ref<MessageEvent | null>(null);
const wsConnected = ref(false);

let wsClient: DeviceWsClient | null = null;

const activeDevice = computed(() =>
  devices.value.find((device) => device.id === activeDeviceId.value) ?? null
);

async function loadDevices(): Promise<void> {
  devices.value = await fetchDevices();
  if (!devices.value.some((device) => device.id === activeDeviceId.value) && devices.value.length > 0) {
    activeDeviceId.value = devices.value[0].id;
  }
}

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

function prependMessage(message: MessageEvent): void {
  messages.value = [message, ...messages.value].slice(0, 12);
}

function upsertDeviceStatus(deviceId: string, status: DeviceSummary["status"]): void {
  devices.value = devices.value.map((device) =>
    device.id === deviceId ? { ...device, status } : device
  );
}

function applyWsEvent(event: WsDeviceEvent): void {
  wsConnected.value = true;

  if (event.type === "session_state") {
    const status = event.payload.status;
    if (status === "connected" || status === "connecting" || status === "disconnected") {
      upsertDeviceStatus(event.deviceId, status);
    }

    const payloadMessage = event.payload.message;
    if (payloadMessage && typeof payloadMessage === "object") {
      const rawMessage = payloadMessage as Record<string, unknown>;
      if (
        typeof rawMessage.id === "string" &&
        typeof rawMessage.timestamp === "string" &&
        typeof rawMessage.direction === "string" &&
        typeof rawMessage.stream === "number" &&
        typeof rawMessage.function === "number" &&
        typeof rawMessage.note === "string"
      ) {
        prependMessage({
          id: rawMessage.id,
          timestamp: rawMessage.timestamp.slice(11, 19) || rawMessage.timestamp,
          direction: rawMessage.direction === "send" ? "send" : "recv",
          stream: rawMessage.stream,
          function: rawMessage.function,
          note: rawMessage.note
        });
      }
    }
    return;
  }

  if (event.type === "secs_message") {
    prependMessage({
      id: typeof event.payload.id === "string" ? event.payload.id : `ws-${Date.now()}`,
      timestamp: typeof event.payload.timestamp === "string"
        ? event.payload.timestamp.slice(11, 19) || event.payload.timestamp
        : new Date().toLocaleTimeString("en-GB", { hour12: false }),
      direction: event.payload.direction === "send" ? "send" : "recv",
      stream: typeof event.payload.stream === "number" ? event.payload.stream : 0,
      function: typeof event.payload.function === "number" ? event.payload.function : 0,
      note: typeof event.payload.note === "string" ? event.payload.note : "Unknown"
    });
  }
}

function reconnectWs(): void {
  wsClient?.disconnect();
  wsConnected.value = false;

  if (!activeDeviceId.value) {
    return;
  }

  wsClient = new DeviceWsClient(activeDeviceId.value, applyWsEvent);
  wsClient.connect();
}

async function runAction(action: "connect" | "disconnect" | "linktest"): Promise<void> {
  if (!activeDevice.value) {
    return;
  }

  const deviceId = activeDevice.value.id;
  const result = action === "connect"
    ? await connectDevice(deviceId)
    : action === "disconnect"
      ? await disconnectDevice(deviceId)
      : await linktestDevice(deviceId);

  ElMessage({
    message: result.message,
    type: result.ok ? "success" : "error"
  });

  await loadDevices();
  await loadMessages();
}

onMounted(async () => {
  await loadDevices();
  await loadMessages();
  reconnectWs();
});

watch(activeDeviceId, async () => {
  await loadMessages();
  reconnectWs();
});

onBeforeUnmount(() => {
  wsClient?.disconnect();
});
</script>

<template>
  <div class="page-shell">
    <header class="hero-card">
      <div>
        <p class="eyebrow">SECS/GEM Host Console</p>
        <h1>Realtime Equipment Console</h1>
        <p class="hero-copy">
          The console now prefers live backend APIs. If the backend is unavailable, it falls back to mock data
          so the page remains explorable during development.
        </p>
      </div>
      <div class="hero-stack">
        <span>Vue 3</span>
        <span>Element Plus</span>
        <span>ECharts</span>
        <span>{{ wsConnected ? "WS Live" : "WS Retry" }}</span>
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
      @connect="runAction('connect')"
      @disconnect="runAction('disconnect')"
      @linktest="runAction('linktest')"
      @refresh="loadMessages"
    />

    <main class="main-grid">
      <DevicePanel
        :devices="devices"
        :active-device-id="activeDeviceId"
        @select="activeDeviceId = $event"
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
            <strong>{{ activeDevice.status }}</strong>
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
