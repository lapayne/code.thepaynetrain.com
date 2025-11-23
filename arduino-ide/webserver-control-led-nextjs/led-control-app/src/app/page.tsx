"use client";
import React, { useState, useEffect, useCallback } from "react";

// Update type definition to include BLINKING
type LedStatus = "ON" | "OFF" | "BLINKING" | "UNKNOWN";

const Home: React.FC = () => {
  // State to track the LED status, using the defined type LedStatus
  const [ledStatus, setLedStatus] = useState<LedStatus>("OFF");
  const [loading, setLoading] = useState<boolean>(false);
  const [error, setError] = useState<string | null>(null);

  // State for the base URL, initialized on the client side only
  const [apiBaseUrl, setApiBaseUrl] = useState<string | null>(null);

  /**
   * Translates the raw numeric state from ESP32 to a user-friendly LedStatus.
   * C++ Status: '0' = OFF, '1' = ON, '2' = BLINKING
   */
  const getStatusFromValue = (statusValue: string): LedStatus => {
    switch (statusValue.trim()) {
      case "0":
        return "OFF";
      case "1":
        return "ON";
      case "2":
        return "BLINKING";
      default:
        return "UNKNOWN";
    }
  };

  /**
   * Fetches the current LED state from the ESP32 via the /STATUS endpoint.
   */
  const fetchStatus = useCallback(
    async (baseUrl: string) => {
      // Keep loading state true only if initial status is unknown
      if (ledStatus === "UNKNOWN") setLoading(true);
      setError(null);
      try {
        // Fetch status, which returns '0', '1', or '2'
        const response = await fetch(`${baseUrl}/STATUS`);
        if (!response.ok) {
          throw new Error(`HTTP error! status: ${response.status}`);
        }
        const statusValue: string = await response.text();
        setLedStatus(getStatusFromValue(statusValue));
      } catch (e: unknown) {
        console.error("Failed to fetch LED status:", e);
        // Only set error if we were ON/BLINKING/OFF and failed to refresh
        if (ledStatus !== "UNKNOWN") {
          setError("Connection error: Could not connect to ESP32 server.");
        }
        setLedStatus("UNKNOWN");
      } finally {
        setLoading(false);
      }
    },
    [ledStatus]
  );

  /**
   * Sends a command to the ESP32 to change the LED state.
   * @param endpoint - The API endpoint to hit (e.g., 'LED_ON', 'LED_OFF', or 'LED_BLINK').
   */
  const sendCommand = async (endpoint: "LED_ON" | "LED_OFF" | "LED_BLINK") => {
    if (!apiBaseUrl) {
      setError("App not initialized. Base URL missing.");
      return;
    }

    setLoading(true);
    setError(null);
    try {
      const response = await fetch(`${apiBaseUrl}/${endpoint}`, {
        method: "POST",
      });
      if (!response.ok) {
        throw new Error(`Failed to send command to ${endpoint}`);
      }
      // Force refresh status after command is sent
      await fetchStatus(apiBaseUrl);
    } catch (error: unknown) {
      console.error("Failed to execute command:", error);
      const message = error instanceof Error ? error.message : "Unknown error";
      setError(`Failed to set LED state: ${message}`);
    } finally {
      setLoading(false);
    }
  };

  // 1. Initial URL Setup (Runs once on mount, client-side only)
  useEffect(() => {
    if (typeof window !== "undefined") {
      // Set the base URL to the origin (the IP of the ESP32)
      setApiBaseUrl(window.location.origin);
    }
  }, []);

  // 2. Data Fetching and Polling (Runs when apiBaseUrl is set)
  useEffect(() => {
    if (!apiBaseUrl) return; // Wait for the URL to be initialized

    // Fetch initial status
    fetchStatus(apiBaseUrl);

    // Set up a polling interval to refresh status every 2 seconds
    const interval = setInterval(() => fetchStatus(apiBaseUrl), 2000);

    // Cleanup function to clear the interval
    return () => clearInterval(interval);
  }, [apiBaseUrl, fetchStatus]);

  // Determine styling based on status (Blue/Green/Teal color scheme)
  const getStatusDisplay = (): { text: string; className: string } => {
    switch (ledStatus) {
      case "ON":
        return {
          text: "Status: ON (Steady)",
          className: "bg-green-100 text-green-800 border-green-400",
        };
      case "OFF":
        return {
          text: "Status: OFF",
          className: "bg-gray-100 text-gray-600 border-gray-300",
        };
      case "BLINKING":
        return {
          text: "Status: BLINKING",
          className: "bg-blue-100 text-blue-800 border-blue-400 animate-pulse", // Added animation for blinking status
        };
      case "UNKNOWN":
      default:
        return {
          text: apiBaseUrl === null ? "Initializing..." : "Connecting...",
          className:
            "bg-yellow-100 text-yellow-800 border-yellow-300 animate-pulse",
        };
    }
  };

  const statusDisplay = getStatusDisplay();

  // Helper for button styling
  const getButtonClass = (
    targetStatus: LedStatus,
    currentStatus: LedStatus,
    baseColor: string,
    hoverColor: string
  ) => {
    const isActive = targetStatus === currentStatus;
    const isDisabled = loading || isActive || apiBaseUrl === null;

    if (isDisabled) {
      return "bg-gray-300 text-gray-500 cursor-not-allowed shadow-inner";
    }
    return `${baseColor} hover:${hoverColor} text-white shadow-lg shadow-opacity-40 transition-all duration-300 ease-in-out transform hover:scale-[1.01]`;
  };

  return (
    <div className="min-h-screen bg-gray-50 flex items-center justify-center p-4">
      <div className="max-w-lg w-full bg-white p-8 rounded-2xl shadow-2xl space-y-8 border border-gray-100">
        <h1 className="text-3xl font-extrabold text-teal-700 border-b-4 border-teal-200 pb-3 mb-4">
          IoT LED State Controller
        </h1>

        {error && (
          <div className="p-3 bg-red-500 text-white rounded-lg shadow-md font-medium">
            Error: {error}
          </div>
        )}

        <div
          className={`status ${statusDisplay.className} p-5 rounded-xl font-extrabold text-xl border-2 transition duration-300 ease-in-out text-center`}
        >
          {statusDisplay.text}
        </div>

        <div className="flex flex-col space-y-4">
          {/* ON Button */}
          <button
            onClick={() => sendCommand("LED_ON")}
            disabled={loading || ledStatus === "ON" || apiBaseUrl === null}
            className={`
              btn w-full py-4 rounded-xl text-lg font-bold ${getButtonClass(
                "ON",
                ledStatus,
                "bg-green-600",
                "bg-green-700"
              )}
            `}
          >
            Turn ON
          </button>

          {/* BLINKING Button (NEW) */}
          <button
            onClick={() => sendCommand("LED_BLINK")}
            disabled={
              loading || ledStatus === "BLINKING" || apiBaseUrl === null
            }
            className={`
              btn w-full py-4 rounded-xl text-lg font-bold ${getButtonClass(
                "BLINKING",
                ledStatus,
                "bg-blue-600",
                "bg-blue-700"
              )}
            `}
          >
            Start BLINKING
          </button>

          {/* OFF Button */}
          <button
            onClick={() => sendCommand("LED_OFF")}
            disabled={loading || ledStatus === "OFF" || apiBaseUrl === null}
            className={`
              btn w-full py-4 rounded-xl text-lg font-bold ${getButtonClass(
                "OFF",
                ledStatus,
                "bg-teal-600",
                "bg-teal-700"
              )}
            `}
          >
            Turn OFF
          </button>
        </div>

        <p className="text-xs text-center text-gray-400 pt-2">
          Base URL: {apiBaseUrl || "Awaiting initialization..."} | Last Poll:{" "}
          {new Date().toLocaleTimeString()}
        </p>
      </div>
    </div>
  );
};

export default Home;
