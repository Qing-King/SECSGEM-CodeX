import type { WsDeviceEvent } from "../types";

export type WsHandler = (message: WsDeviceEvent) => void;

function buildWebSocketUrl(deviceId: string): string {
  const protocol = window.location.protocol === "https:" ? "wss:" : "ws:";
  return `${protocol}//${window.location.host}/ws/devices/${deviceId}`;
}

export class DeviceWsClient {
  private readonly deviceId: string;
  private readonly handler: WsHandler;
  private socket: WebSocket | null = null;

  constructor(deviceId: string, handler: WsHandler) {
    this.deviceId = deviceId;
    this.handler = handler;
  }

  connect(): void {
    this.disconnect();

    try {
      this.socket = new WebSocket(buildWebSocketUrl(this.deviceId));
      this.socket.onmessage = (event) => {
        try {
          const payload = JSON.parse(event.data) as WsDeviceEvent;
          this.handler(payload);
        } catch {
          // Ignore malformed frames from the minimal backend.
        }
      };
      this.socket.onerror = () => {
        this.disconnect();
      };
    } catch {
      this.disconnect();
    }
  }

  disconnect(): void {
    if (this.socket) {
      this.socket.close();
      this.socket = null;
    }
  }
}
