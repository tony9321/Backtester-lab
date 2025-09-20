# Google Cloud Run Strategy Optimizer 🚀

Your C++ strategy optimizer is now ready for deployment to Google Cloud Run!

## 🏗️ Architecture Overview

```
┌─────────────────┐    HTTPS API    ┌─────────────────┐    Native Exec    ┌─────────────────┐
│   React Native  │ ────────────────▶│  Cloud Run API  │ ────────────────▶│  C++ Optimizer  │
│     Frontend    │                 │  Express Server │                  │     Binary      │
└─────────────────┘                 └─────────────────┘                  └─────────────────┘
```

## 🚀 Deployment Steps

### 1. Set up Google Cloud Project
```bash
# Install gcloud CLI if needed
# https://cloud.google.com/sdk/docs/install

# Authenticate with Google Cloud
gcloud auth login

# Create a new project (or use existing)
gcloud projects create your-project-id
gcloud config set project your-project-id

# Enable billing (required for Cloud Run)
# https://console.cloud.google.com/billing
```

### 2. Deploy to Cloud Run
```bash
# Navigate to the root directory
cd /workspaces/Backtester-lab

# Run deployment script
./deploy.sh
```

### 3. Update React Native App
```bash
# In TradingDashboard directory
cd TradingDashboard

# Update environment variable with your Cloud Run URL
echo 'EXPO_PUBLIC_API_URL="https://your-service-url"' > .env

# The service URL will be printed after deployment
```

## 📋 API Endpoints

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/health` | GET | Health check |
| `/info` | GET | API information |
| `/optimize` | POST | Run optimization |

### Optimization Request Format
```json
{
  "symbol": "AAPL",
  "startDate": "2024-01-01", 
  "endDate": "2024-12-31",
  "parameters": {
    "fastPeriod": 12,
    "slowPeriod": 26,
    "signalPeriod": 9
  }
}
```

## 🔧 Configuration

### Environment Variables
- `NODE_ENV=production` - Production mode
- `PORT=8080` - Server port (automatically set by Cloud Run)

### Resource Limits
- **Memory**: 4GB
- **CPU**: 2 vCPUs  
- **Timeout**: 60 minutes
- **Max Instances**: 10

## 💰 Cost Estimation

| Usage Level | Monthly Cost |
|-------------|--------------|
| Light (100 requests/day) | $2-5 |
| Medium (1000 requests/day) | $10-25 |
| Heavy (10000 requests/day) | $50-100 |

## 🧪 Testing

```bash
# Test health check
curl https://your-service-url/health

# Test API info
curl https://your-service-url/info

# Test optimization (POST request)
curl -X POST https://your-service-url/optimize \
  -H "Content-Type: application/json" \
  -d '{
    "symbol": "AAPL",
    "startDate": "2024-01-01",
    "endDate": "2024-12-31", 
    "parameters": {
      "fastPeriod": 12,
      "slowPeriod": 26,
      "signalPeriod": 9
    }
  }'
```

## 🔍 Monitoring

View logs and metrics:
```bash
# View logs
gcloud run logs tail strategy-optimizer --region us-central1

# View in Cloud Console
# https://console.cloud.google.com/run
```

## 🚨 Troubleshooting

### Common Issues

1. **Build Errors**: Check C++ dependencies in the Dockerfile
2. **Memory Issues**: Increase memory allocation in deploy script  
3. **Timeout**: Adjust timeout settings for long optimizations
4. **Authentication**: Ensure gcloud is properly authenticated

### Debug Commands
```bash
# Check deployment status
gcloud run services describe strategy-optimizer --region us-central1

# View recent logs
gcloud run logs tail strategy-optimizer --region us-central1

# Test locally first
docker build -t strategy-optimizer .
docker run -p 8080:8080 strategy-optimizer
```

## 📱 React Native Integration

The `TradingDashboard` app is already configured to work with Cloud Run:

- ✅ Firebase removed completely
- ✅ Cloud Run API client ready
- ✅ Environment variables configured
- ✅ TypeScript errors resolved

Just update the `EXPO_PUBLIC_API_URL` after deployment!

## 🎯 Next Steps

1. **Deploy**: Run `./deploy.sh`
2. **Test**: Verify all endpoints work
3. **Update App**: Set Cloud Run URL in React Native
4. **Monitor**: Check logs and performance
5. **Scale**: Adjust resources as needed

Your C++ strategy optimizer will now run natively in the cloud with full performance! 🚀