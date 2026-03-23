import type {
  ActionApiResponse,
  DeviceApiResponse,
  DeviceSummary,
  MessageEvent
} from "../types";
import { deviceList, recentMessages } from "../mock-data";

async function parseJson<T>(response: Response): Promise<T> {
  if (!response.ok) {
    throw new Error(`HTTP ${response.status}`);
  }
  return response.json() as Promise<T>;
}

export async function fetchDevices(): Promise<DeviceSummary[]> {
  try {
    const response = await fetch("/api/devices", {
      headers: { Accept: "application/json" }
    });
    const payload = await parseJson<DeviceApiResponse>(response);
    return payload.items;
  } catch {
    return deviceList;
  }
}

export async function fetchRecentMessages(deviceId: string): Promise<MessageEvent[]> {
  try {
    const response = await fetch(`/api/devices/${deviceId}/messages`, {
      headers: { Accept: "application/json" }
    });
    return await parseJson<MessageEvent[]>(response);
  } catch {
    return recentMessages;
  }
}

async function postDeviceAction(deviceId: string, action: string): Promise<ActionApiResponse> {
  const fallback: ActionApiResponse = {
    ok: true,
    action,
    deviceId,
    message: `Mock ${action} accepted`
  };

  try {
    const response = await fetch(`/api/devices/${deviceId}/${action}`, {
      method: "POST",
      headers: { Accept: "application/json" }
    });
    return await parseJson<ActionApiResponse>(response);
  } catch {
    return fallback;
  }
}

export function connectDevice(deviceId: string): Promise<ActionApiResponse> {
  return postDeviceAction(deviceId, "connect");
}

export function disconnectDevice(deviceId: string): Promise<ActionApiResponse> {
  return postDeviceAction(deviceId, "disconnect");
}

export function linktestDevice(deviceId: string): Promise<ActionApiResponse> {
  return postDeviceAction(deviceId, "linktest");
}

