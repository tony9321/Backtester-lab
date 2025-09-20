# Google Cloud Setup Instructions 🌟

## Step 1: Enable Billing (Required) 💳

Cloud Run requires a billing account to be linked to your project. Don't worry - it has a generous free tier:

### **Free Tier for Cloud Run:**
- **2 million requests** per month
- **400,000 GB-seconds** of memory per month  
- **200,000 vCPU-seconds** per month
- **1GB network egress** per month from North America

For your strategy optimizer usage, you'll likely stay within the free tier!

### **Enable Billing:**

1. **Go to Google Cloud Console:**
   ```
   https://console.cloud.google.com/billing
   ```

2. **Create a billing account** (if you don't have one):
   - Click "CREATE BILLING ACCOUNT"
   - Enter your payment information
   - Google requires a payment method but won't charge unless you exceed free limits

3. **Link billing to your project:**
   - Select project: `backtester-strategy-optimizer`  
   - Link your billing account

### **Alternative: Use Existing Project**
If you have another Google Cloud project with billing enabled, you can use that instead:

```bash
# List your projects
gcloud projects list

# Set existing project
gcloud config set project YOUR-EXISTING-PROJECT-ID
```

## Step 2: Continue After Billing Setup ✅

Once billing is enabled, run:

```bash
# Enable required APIs
gcloud services enable run.googleapis.com cloudbuild.googleapis.com

# Deploy your service
./deploy.sh
```

## Cost Estimate for Your Use Case 📊

**Scenario: 100 optimizations per day**
- Each optimization: ~30 seconds, 1GB memory
- Monthly usage: ~3,000 requests, ~1,500 GB-seconds
- **Cost: $0 (within free tier)** 🎉

**Scenario: 1,000 optimizations per day**  
- Monthly usage: ~30,000 requests, ~15,000 GB-seconds
- **Cost: ~$5-10/month** 💰

Much better than Firebase Functions! 🚀

---

**Next Steps:**
1. ✅ Enable billing at https://console.cloud.google.com/billing
2. ✅ Come back and run the deployment
3. ✅ Test your C++ optimizer in the cloud!