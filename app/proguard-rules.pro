# MineMc ProGuard rules
# Keep native bridge intact
-keep class com.ryzix.minemc.NativeBridge { *; }
-keep class com.ryzix.minemc.** { *; }

# Suppress warnings for unused NDK stubs
-dontwarn android.graphics.**
