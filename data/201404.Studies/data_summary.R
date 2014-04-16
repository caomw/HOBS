## setwd("~/repos/CS294-84Project/data/201404.Studies")

d <- read.table("aggregate.txt")
colnames(d) <- c("participant", "trial", "target", "first", "visual_search", "tap_time", "complete", "group", "miss_count")
## for a quick summary
intensity <- d[d$group =="intensity", ]
name <- d[d$group =="name", ]

intensity_time <- ggplot(intensity, aes(x = complete, fill = is.na(first))) +
  geom_density(binwidth = 0.1, alpha = 0.5) +
  xlab("distribution of acquisition time") +
  ggtitle("with IR intensity") + 
  theme_bw() +
  scale_fill_discrete(name="w/o disambiguation",
                      labels=c("disambiguation", "direct"))
name_time <- ggplot(name, aes(x = complete, fill = is.na(first))) + 
  geom_density(binwidth = 0.1, alpha = 0.5) +
  xlab("distribution of acquisition time") +
  ggtitle("without IR intensity") + 
  theme_bw() + 
  scale_fill_discrete(name="w/o disambiguation",
                      labels=c("disambiguation", "direct"))

pdf("figures/single_vs_multiple.pdf", width = 7, height = 6)
multiplot(intensity_time, name_time, cols = 1)
dev.off()

## 41% wrong rate for guessing
intensity_multiple <- intensity[!is.na(intensity$first), ]
wrong_guess <- sum(intensity_multiple$first != intensity_multiple$target) 
wrong_percentage <- sum(intensity_multiple$first != intensity_multiple$target) / length(intensity_multiple$target)
ceiling(c(nrow(intensity), nrow(intensity_multiple), wrong_guess, wrong_percentage * 100))

## 63% wrong rate for guessing
name_multiple <- name[!is.na(name$first), ]
wrong_guess <- sum(name_multiple$first != name_multiple$target) 
wrong_percentage <- sum(name_multiple$first != name_multiple$target) / length(name_multiple$target)
ceiling(c(nrow(name), nrow(name_multiple), wrong_guess, wrong_percentage * 100))

## this is for individuals
for (i in 1:5) {
  user_multiple <- name[name$participant == user, ]
  name_multiple <- user_multiple[!is.na(user_multiple$first), ]
  wrong_guess <- sum(name_multiple$first != name_multiple$target) 
  wrong_percentage <- sum(name_multiple$first != name_multiple$target) / length(name_multiple$target)
  cat( paste( ceiling(c(nrow(user_multiple), nrow(name_multiple), wrong_guess, wrong_percentage * 100)), "\n") )
}

multiples <- rbind(intensity_multiple, name_multiple)
disambiguation_needed <- ggplot(multiples, aes(x = complete, colour = group)) +
  stat_ecdf() +
  xlab("cdf of acquisition time") +
  ggtitle("when disambiguation is involved") + 
  theme_bw()
disambiguation_needed


## the following graph is what we are using for the paper

for (i in 1:nrow(d)) {
  if (toString(d$group[i]) == "intensity") {
    d$type[i] <- "Intensity IR"
  }
  else {
    d$type[i] <- "Naive IR"
  }
}

summary(d[d$type == "Intensity IR",])
summary(d[d$type == "Intensity IR",])

## Intensity IR
Median : 2.964
Mean   : 3.641
## Naive IR
Median : 3.5317
Mean   : 4.3113

box1 <- ggplot(d, aes(factor(type), complete, fill = d$type)) +
  geom_boxplot(lwd=0.5) + ylim(2.5,22) + xlab("") + ylab("time (seconds)") +
##  scale_fill_manual(values = c("#A4C3D9", "#9FB987")) + 
  stat_summary(fun.y = "mean", geom = "text", label="---", size= 8, color= "white") +
  annotate("text", x = 1, y = 22, label = "mean: 3.64", size=3.6) +
  annotate("text", x = 1, y = 21, label = "median: 2.96", size=3.6) +
  annotate("text", x = 2, y = 22, label = "mean: 4.31", size=3.6) +
  annotate("text", x = 2, y = 21, label = "median: 3.53", size=3.6) +
  theme_bw() +
  theme(legend.position = "none", axis.title.y=element_blank())

overall_time <- ggplot(d, aes(x = complete, colour = type, shape = type)) +
  stat_ecdf() +
  ylab("cumulative distribution function") +
  xlab("acquisition time") + 
  theme_bw() + 
  theme(legend.justification=c(1,0), legend.position=c(1,0))

overall_time

pdf("figures/iteraction2_study.pdf", width = 7, height = 4)
multiplot(box1, overall_time, cols = 2)
dev.off()

## the following are not used for now
pdf("figures/disambiguation_comparison.pdf", width = 7, height = 6)
disambiguation_needed
dev.off()

intensity[intensity$miss_count > 0, c(1, 2, 3, 4, 8, 9)]
name[name$miss_count > 0, c(1, 2, 3, 4, 8, 9)]

sum(name$miss_count)
sum(intensity$miss_count)


## these two are just for quick visualizations
intensity_by_targets <- ggplot(intensity, aes(x = complete, fill = is.na(first))) +
  geom_density(binwidth = 0.5, alpha = 0.4, aes(y=..count..)) +
  facet_grid(. ~ target) +
  xlab("distribution of acquisition time") +
  ggtitle("with IR intensity") + 
  theme_bw() +
  scale_fill_discrete(name="w/o disambiguation",
                      labels=c("disambiguation", "direct"))
name_by_targets <- ggplot(name, aes(x = complete, fill = is.na(first))) +
  geom_density(binwidth = 0.5, alpha = 0.4, aes(y=..count..)) +
  facet_grid(. ~ target) + 
  xlab("distribution of acquisition time") +
  ggtitle("without IR intensity") + 
  theme_bw() +
  scale_fill_discrete(name="w/o disambiguation",
                      labels=c("disambiguation", "direct"))
pdf("figures/disambiguation_by_targets.pdf", height = 6, width = 11)
multiplot(intensity_by_targets, name_by_targets, cols = 1)
dev.off()

## we only need name and intensity for this part of timing analysis
library(reshape)
name_data <- melt(name, measure.var = c("visual_search", "tap_time", "complete"))
intensity_data <- melt(intensity, measure.var = c("visual_search", "tap_time", "complete"))

f1 <- ggplot(name_data, aes(x = value, fill = variable)) +
  geom_density(alpha = 0.3) +
  xlab("distribution of acquisition time") +
  ggtitle("without IR intensity") + 
  theme_bw()
f2 <- ggplot(intensity_data, aes(x = value, fill = variable)) +
  geom_density(alpha = 0.3) +
  xlab("distribution of acquisition time") +
  ggtitle("with IR intensity") + 
  theme_bw()

pdf("figures/timing_model.pdf", width = 8, height = 7)
multiplot(f1, f2, cols = 1)
dev.off()



t.test(name$complete, intensity$complete)



## intensity plot
measure <- read.table(file = "intensity.dat", row.names = 1, header = T)

library(reshape)
new <- melt(measure)
distance <- rep(1:8, each = 13)
offset <- rep(c(0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60), times = 8)
df <- cbind(new, offset, distance)

df_mirror <- df[df$offset > 0,]
df_mirror$offset <- -df_mirror$offset
whole <- rbind(df, df_mirror)

p <- ggplot(whole, aes(x=distance, y=offset, colour = value)) +
  geom_point(aes(size= log(value))) +
  theme_bw() +
  theme(legend.position="none") +
  xlab("distance (m) between IR and the receiver") +
  ylab("offset (cm) from the center") +
  ggtitle("IR intensity distribution (measured)") +
  scale_fill_brewer() +
  xlim(0, 8.1)
pdf("figures/IRIntensityDistribution.pdf", width = 10, height = 3)
p
dev.off()
