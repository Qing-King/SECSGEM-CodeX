import type { AlarmItem, DeviceSummary, MessageEvent, RuntimeMetric } from "./types";

export const deviceList: DeviceSummary[] = [
  {
    id: "eqp01",
    name: "EQP-01 Etcher",
    ip: "10.10.20.15",
    port: 5000,
    mode: "active",
    status: "connected",
    selected: true
  },
  {
    id: "eqp02",
    name: "EQP-02 Cleaner",
    ip: "0.0.0.0",
    port: 5001,
    mode: "passive",
    status: "disconnected",
    selected: false
  }
];

export const runtimeMetrics: RuntimeMetric[] = [
  { label: "Online Devices", value: "1 / 2" },
  { label: "Messages Today", value: "1,284" },
  { label: "Active Alarms", value: "3" },
  { label: "Round Trip", value: "42 ms" }
];

export const recentMessages: MessageEvent[] = [
  {
    id: "m1",
    timestamp: "10:00:01",
    direction: "send",
    stream: 1,
    function: 13,
    note: "Establish Communications Request"
  },
  {
    id: "m2",
    timestamp: "10:00:01",
    direction: "recv",
    stream: 1,
    function: 14,
    note: "Establish Communications Acknowledge"
  },
  {
    id: "m3",
    timestamp: "10:00:05",
    direction: "recv",
    stream: 6,
    function: 11,
    note: "Event Report"
  }
];

export const alarmList: AlarmItem[] = [
  {
    id: "a1",
    code: "ALM-001",
    level: "critical",
    message: "Vacuum low threshold exceeded."
  },
  {
    id: "a2",
    code: "ALM-007",
    level: "warning",
    message: "Door interlock transition pending clear."
  },
  {
    id: "a3",
    code: "ALM-011",
    level: "info",
    message: "Recipe validation reminder."
  }
];

