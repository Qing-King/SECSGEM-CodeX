export interface WsEnvelope {
  type: string;
  payload: unknown;
}

export type WsHandler = (message: WsEnvelope) => void;

export class MockWsClient {
  private readonly handler: WsHandler;

  constructor(handler: WsHandler) {
    this.handler = handler;
  }

  connect(): void {
    setTimeout(() => {
      this.handler({
        type: "secs_message",
        payload: {
          direction: "recv",
          stream: 6,
          function: 11,
          note: "Mock event from WebSocket"
        }
      });
    }, 1200);
  }
}
