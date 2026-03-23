import type { DeviceApiResponse, DeviceSummary, MessageEvent } from "../types";
import { deviceList, recentMessages } from "../mock-data";

export async function fetchDevices(): Promise<DeviceSummary[]> {
  const response: DeviceApiResponse = {
    items: deviceList
  };
  return Promise.resolve(response.items);
}

export async function fetchRecentMessages(deviceId: string): Promise<MessageEvent[]> {
  void deviceId;
  return Promise.resolve(recentMessages);
}
