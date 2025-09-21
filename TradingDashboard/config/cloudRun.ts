// Cloud Run API Configuration
// Replace with your actual Cloud Run service URL after deployment

export const CLOUD_RUN_CONFIG = {
  // This will be your Cloud Run service URL
  // Format: https://SERVICE_NAME-HASH-uc.a.run.app
  API_BASE_URL: process.env.EXPO_PUBLIC_API_URL || 'http://localhost:8080',
  
  // API endpoints
  ENDPOINTS: {
    HEALTH: '/health',
    INFO: '/info', 
    OPTIMIZE: '/optimize'
  },
  
  // Request configuration
  DEFAULT_TIMEOUT: 900000, // 15 minutes for optimization requests
  MAX_RETRIES: 3
};

// Helper function to get full endpoint URL
export const getEndpointUrl = (endpoint: keyof typeof CLOUD_RUN_CONFIG.ENDPOINTS): string => {
  return `${CLOUD_RUN_CONFIG.API_BASE_URL}${CLOUD_RUN_CONFIG.ENDPOINTS[endpoint]}`;
};

// API health check
export const checkApiHealth = async (): Promise<boolean> => {
  try {
    const response = await fetch(getEndpointUrl('HEALTH'), {
      method: 'GET',
      headers: {
        'Content-Type': 'application/json'
      }
    });
    return response.ok;
  } catch (error) {
    console.error('API health check failed:', error);
    return false;
  }
};