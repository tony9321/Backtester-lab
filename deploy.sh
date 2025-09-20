#!/bin/bash

# Google Cloud Run Deployment Script
set -e

echo "🚀 Deploying Strategy Optimizer to Google Cloud Run..."

# Configuration
PROJECT_ID=$(gcloud config get-value project 2>/dev/null)
if [ -z "$PROJECT_ID" ]; then
    echo "❌ No active Google Cloud project found. Please run:"
    echo "gcloud config set project YOUR-PROJECT-ID"
    exit 1
fi
SERVICE_NAME="strategy-optimizer"
REGION="us-central1"

echo "📋 Deployment Configuration:"
echo "Project ID: ${PROJECT_ID}"
echo "Service Name: ${SERVICE_NAME}"
echo "Region: ${REGION}"
echo ""

# Check if gcloud is authenticated
if ! gcloud auth list --filter=status:ACTIVE --format="value(account)" | grep -q "@"; then
    echo "❌ Not authenticated with gcloud. Please run:"
    echo "gcloud auth login"
    exit 1
fi

# Set the project
gcloud config set project ${PROJECT_ID}

# Enable required APIs
echo "🔧 Enabling required APIs..."
gcloud services enable run.googleapis.com
gcloud services enable cloudbuild.googleapis.com

# Build and deploy to Cloud Run
echo "🏗️  Building and deploying to Cloud Run..."
gcloud run deploy ${SERVICE_NAME} \
  --source . \
  --platform managed \
  --region ${REGION} \
  --allow-unauthenticated \
  --memory 4Gi \
  --cpu 2 \
  --timeout 3600s \
  --max-instances 10 \
  --set-env-vars NODE_ENV=production

# Get the service URL
SERVICE_URL=$(gcloud run services describe ${SERVICE_NAME} --platform managed --region ${REGION} --format 'value(status.url)')

echo ""
echo "✅ Deployment completed successfully!"
echo ""
echo "🌐 Your API is now live at:"
echo "${SERVICE_URL}"
echo ""
echo "📋 Endpoints:"
echo "Health Check: ${SERVICE_URL}/health"
echo "API Info: ${SERVICE_URL}/info"
echo "Optimization: ${SERVICE_URL}/optimize (POST)"
echo ""
echo "🧪 Test your API:"
echo "curl ${SERVICE_URL}/health"
echo ""
echo "📱 Update your React Native app:"
echo "export EXPO_PUBLIC_API_URL=\"${SERVICE_URL}\""

# Save the URL to a file
echo ${SERVICE_URL} > cloud-run-url.txt
echo "💾 Service URL saved to cloud-run-url.txt"