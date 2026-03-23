export type DeviceMode = "active" | "passive";
export type DeviceStatus = "connected" | "connecting" | "disconnected";

export interface DeviceSummary {
  id: string;
  name: string;
  ip: string;
  port: number;
  mode: DeviceMode;
  status: DeviceStatus;
  selected: boolean;
}

export interface RuntimeMetric {
  label: string;
  value: string;
}

export interface MessageEvent {
  id: string;
  timestamp: string;
  direction: "send" | "recv";
  stream: number;
  function: number;
  note: string;
}

export interface AlarmItem {
  id: string;
  code: string;
  level: "critical" | "warning" | "info";
  message: string;
}

export interface DeviceApiResponse {
  items: DeviceSummary[];
}

export interface WsDeviceEvent {
  type: "session_state" | "secs_message" | "alarm" | "event";
  deviceId: string;
  timestamp: string;
  payload: Record<string, unknown>;
}

