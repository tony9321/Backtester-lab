# TradingDashboard - Strategy Optimization System

## Architecture

```
React Native App (Expo)
    ↓ Direct HTTPS Request
Google Cloud Run Container
    ├── Express.js API Server
    ├── Native C++ Strategy Optimizer
    └── All Required Libraries
    ↓ External API
Alpaca Markets API
```

## Current Project Structure

```
TradingDashboard/
├── components/
│   ├── Dashboard.tsx            # Main dashboard (Cloud Run ready)
│   ├── ParameterControls.tsx    # User input controls
│   ├── OptimizationCard.tsx     # Result display cards
│   └── MetricsChart.tsx         # Performance charts
├── config/
│   └── cloudRun.ts             # Cloud Run API configuration
├── services/
│   └── optimizationService.ts   # Cloud Run API service
├── types/
│   └── trading.ts              # TypeScript definitions
├── App.tsx                     # Main app component
└── package.json                # Clean dependencies (no Firebase)
```

## 🔧 Configuration

Update your Cloud Run API URL in `config/cloudRun.ts`:

```typescript
export const CLOUD_RUN_CONFIG = {
  API_BASE_URL: 'https://your-service-xyz.run.app',
  // ... other configuration
};
```