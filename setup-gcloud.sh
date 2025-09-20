#!/bin/bash

# Google Cloud Setup Script
# Run this to set up your Google Cloud environment

set -e

echo "🚀 Setting up Google Cloud for Strategy Optimizer deployment..."

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Check if gcloud is installed
if ! command -v gcloud &> /dev/null; then
    echo -e "${RED}❌ gcloud CLI not found!${NC}"
    echo "Install it from: https://cloud.google.com/sdk/docs/install"
    echo ""
    echo "For Linux/macOS:"
    echo "curl https://sdk.cloud.google.com | bash"
    echo "exec -l \$SHELL"
    exit 1
fi

echo -e "${GREEN}✅ gcloud CLI found${NC}"

# Check if user is authenticated
if ! gcloud auth list --filter=status:ACTIVE --format="value(account)" | grep -q .; then
    echo -e "${YELLOW}🔐 Authenticating with Google Cloud...${NC}"
    gcloud auth login
fi

# Set or create project
echo -e "${YELLOW}📋 Setting up Google Cloud project...${NC}"
echo ""
echo "Enter your Google Cloud Project ID (or press Enter to create a new one):"
read -r PROJECT_ID

if [ -z "$PROJECT_ID" ]; then
    PROJECT_ID="strategy-optimizer-$(date +%s)"
    echo -e "${YELLOW}📝 Creating new project: $PROJECT_ID${NC}"
    gcloud projects create $PROJECT_ID
fi

# Set the project
gcloud config set project $PROJECT_ID
echo -e "${GREEN}✅ Using project: $PROJECT_ID${NC}"

# Enable required APIs
echo -e "${YELLOW}⚙️  Enabling required APIs...${NC}"
gcloud services enable run.googleapis.com
gcloud services enable containerregistry.googleapis.com
gcloud services enable cloudbuild.googleapis.com

# Set default region
echo -e "${YELLOW}🌍 Setting default region to us-central1...${NC}"
gcloud config set run/region us-central1

echo ""
echo -e "${GREEN}🎉 Google Cloud setup complete!${NC}"
echo ""
echo -e "${YELLOW}📋 Your setup:${NC}"
echo "Project ID: $PROJECT_ID"
echo "Region: us-central1"
echo "APIs enabled: Cloud Run, Container Registry, Cloud Build"
echo ""
echo -e "${GREEN}✅ Ready for deployment!${NC}"

# Save project info
echo "PROJECT_ID=$PROJECT_ID" > .env.gcloud
echo "REGION=us-central1" >> .env.gcloud
echo "PROJECT setup saved to .env.gcloud"