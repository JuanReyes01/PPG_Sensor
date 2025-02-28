# ppg
library(seewave)
library(tuneR)
library(tidyverse)

######## read data
ppg <- fread("C:/Users/ehumre/Documents/GitHub/PPG_Sensor/ppg_data.csv")
ppg <- fread("C:/Users/ehumre/Dropbox/MPI/Wingbeat/Colombia25/PPG/20250226/ppg_data_hummingbird2_trial2.csv")
# Be sure to move this file to somewhere you want to save it.
# It will be erased if you update the github repository

######## set sampling rate (I'm not sure what the true sampling rate is...)
# diff(ppg$`Time (s)`) %>% hist(xlim = c(0, 0.015), breaks = 10000)
# 1/median(diff(ppg$`Time (s)`))
# 1/mean(diff(ppg$`Time (s)`))
sampling_rate <- 100 # Hz

######## View full data
plot(ppg$`Time (s)`, ppg$`PPG Value`, type = "l")

### View first few sec of data (0 to 5 seconds)
plot(ppg$`Time (s)`, ppg$`PPG Value`, type = "l", xlim = c(0, 5))

## cut duplicated data from the file
ppg <- ppg %>% filter(`Time (s)` > 4)


######## convert data to audio
w <- Wave(left = ppg$`PPG Value`, samp.rate = sampling_rate, bit = 8)
plot(w)

######## filter for frequencies below 0.5 Hz (30 BPM) or above 20 Hz (1200 BPM)
bw <- bwfilter(w, f = sampling_rate, from = 0.5, to = 40)

######## View spectrogram, i.e. frequency of heart rate over time
spectro(wave = bw, #FFT_size = sampling_rate * window,
        flim = c(0, 0.02),
        f = sampling_rate,
        plot = TRUE, scale = FALSE)

######## get precise frequency measurement
density <- meanspec(bw, f = sampling_rate, plot = FALSE)
peaks <- fpeaks(density, f = sampling_rate, nmax = 5, plot = TRUE)
peaks

######## choose highest amplitude signal
hb_freq <- peaks[which(peaks[,2] == 1),1] * 1000
print(paste0("BPM: ", round(hb_freq*60, 1)))

# smooth data to get regular sampling intervals
# s <- smooth.spline(x = ppg$`Time (s)`, y = ppg$`PPG Value`, spar = 0.01)
# time <- seq(0, max(ppg$`Time (s)`))
