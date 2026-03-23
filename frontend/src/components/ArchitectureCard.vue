<template>
  <section class="card">
    <div class="card-header">
      <h2>System Architecture</h2>
      <span>Frontend / Backend Contract</span>
    </div>
    <div class="architecture-grid">
      <div class="architecture-item">
        <span>Frontend</span>
        <strong>Vue 3 + Element Plus + ECharts</strong>
        <p>Renders equipment status, connection tools, alarms, messages, and operator views.</p>
      </div>
      <div class="architecture-item">
        <span>Backend</span>
        <strong>C++ Host Service</strong>
        <p>Owns HSMS, SECS-II, GEM workflows, REST endpoints, and WebSocket fan-out.</p>
      </div>
      <div class="architecture-item architecture-item-wide">
        <span>Contract</span>
        <strong>REST + WebSocket</strong>
        <pre class="code-block">GET /api/devices
GET /api/devices/{id}/messages
POST /api/devices/{id}/connect
WS /ws/devices/{id}</pre>
      </div>
    </div>
  </section>
</template>

