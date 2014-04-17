setwd("~/repos/CS294-84Project/data/201309.Studies/script/")
require('ggplot2')

## to run this script, you need multiplot

## Summarizes data.
## Gives count, mean, standard deviation, standard error of the mean, and confidence interval (default 95%).
##   data: a data frame.
##   measurevar: the name of a column that contains the variable to be summariezed
##   groupvars: a vector containing names of columns that contain grouping variables
##   na.rm: a boolean that indicates whether to ignore NA's
##   conf.interval: the percent range of the confidence interval (default is 95%)
summarySE <- function(data=NULL, measurevar, groupvars=NULL, na.rm=FALSE, conf.interval=.95, .drop=TRUE) {
    require(plyr)

    # New version of length which can handle NA's: if na.rm==T, don't count them
    length2 <- function (x, na.rm=FALSE) {
        if (na.rm) sum(!is.na(x))
        else       length(x)
    }

    # This is does the summary; it's not easy to understand...
    datac <- ddply(data, groupvars, .drop=.drop,
                   .fun= function(xx, col, na.rm) {
                           c( N    = length2(xx[,col], na.rm=na.rm),
                              mean = mean   (xx[,col], na.rm=na.rm),
                              sd   = sd     (xx[,col], na.rm=na.rm)
                              )
                          },
                    measurevar,
                    na.rm
             )

    # Rename the "mean" column    
    datac <- rename(datac, c("mean"=measurevar))

    datac$se <- datac$sd / sqrt(datac$N)  # Calculate standard error of the mean

    # Confidence interval multiplier for standard error
    # Calculate t-statistic for confidence interval: 
    # e.g., if conf.interval is .95, use .975 (above/below), and use df=N-1
    ciMult <- qt(conf.interval/2 + .5, datac$N-1)
    datac$ci <- datac$se * ciMult

    return(datac)
}


## read data in first
df <- read.csv("data.csv")

## discard data frmo valkerie and Micheal
df <- df[!is.na(df$correct_time),]
df <- df[!(grepl("valkerie", df$participant) | grepl("Micheal", df$participant)), ]

df.IR.single <- df[grepl("IR", df$mode) & grepl("single", df$correct_type), ]
df.IR.mul <- df[grepl("IR", df$mode) & grepl("mul", df$correct_type), ]

#### draw the list mode, time vs. position
## notice this is no longer reproducible for the graph we use in the paper
## due to my stupid usage of git

df.List <- df[grepl("List", df$mode), ]
df.List <- df.List[!grepl("next", df.List$fate),]
# mapping from id to name
name <- "GOWFJSYRAM"
df.List$target <- sapply(df.List$target, function(x) substr(name, x, x))
dfc <- summarySE(df.List, measurevar="correct_time", groupvars = "target", na.rm=TRUE)
label.x <- sapply(names(table(df.List$target)), function(x) regexpr(x, name))
# Standard error of the mean
cols <- c("LINE1"="#56B4E9","LINE2"="#3591d1","BAR"="#62c76b")

scalability_study <- ggplot(dfc, aes(x=target, y=correct_time, group=1)) + 
  geom_line(lwd=0.5, alpha=0.5) +
  geom_errorbar(aes(ymin=correct_time-se, ymax=correct_time+se), width=.1) +
  geom_point(shape=21, size=3, fill="white") +
  geom_abline(intercept = 5.08526, slope = 0.59287, lwd=0.5, alpha=0.7, linetype=3) +
  geom_hline(lwd=0.5, colour = "#56B4E9" , yintercept=9.16, linetype=1) +
geom_hline(lwd=0.5, colour = "#56B4E9", yintercept=6.40, linetype=1) +
annotate("text", x = 1.3, y = 9.6, colour = "#56B4E9", label = "with refinement", size=4) +
annotate("text", x = 1, y = 7.16, colour = "#56B4E9", label = "without\n refinement", size=4) +
ylim(3,14) + xlab("order in list") + ylab("time (seconds)") + scale_x_discrete(labels=seq(1, 10)) +
  scale_colour_manual(name="Error Bars",values=cols) + scale_fill_manual(name="Bar",values=cols) +
  theme(legend.position = c(.5, .5)) +
  theme_bw()

pdf("R_List_by_Target.pdf", width=7, height=4)
scalability_study
dev.off()

## the following box is not that useful
box <- ggplot(df.List, aes(factor(target), correct_time))
box <- box + geom_boxplot(lwd=0.5, fill = '#9FB987', alpha=0.3) + ylim(2.5,15) + xlab("order in list") + ylab("time (seconds)") + scale_x_discrete(labels=seq(0, 9))
box <- box +
    geom_hline(lwd=0.5, fill='black', yintercept=9.16) +
    geom_hline(lwd=0.5, fill='black', yintercept=6.40) +
    geom_abline(intercept = 5.74455, slope = 0.51521, lwd=0.5) +
##    stat_summary(fun.y = "median", geom = "text", label="---", size= 6, color= "blue") +
    stat_summary(fun.data = function(x) c(y = 2.6, label = round(mean(x),2)), geom = "text", size = 4, fun.y = mean)
## linear fit
sorted.name <- sort(unique(df.List$target))
target1 <- sapply(df.List$target, function(x) which(sorted.name == x) - 1)
fit <- lm(df.List$correct_time ~ target1)
box + theme(legend.position = "none") 
dev.off()

## Call:
## lm(formula = dfc$correct_time ~ seq(1, 10))

## Residuals:
##     Min      1Q  Median      3Q     Max 
## -0.8289 -0.4426 -0.3235  0.4911  1.1009 

## Coefficients:
##             Estimate Std. Error t value Pr(>|t|)    
## (Intercept)  5.08526    0.49923  10.186 7.40e-06 ***
## seq(1, 10)   0.59287    0.08046   7.369 7.85e-05 ***
## ---
## Signif. codes:  0 ‘***’ 0.001 ‘**’ 0.01 ‘*’ 0.05 ‘.’ 0.1 ‘ ’ 1 

## Residual standard error: 0.7308 on 8 degrees of freedom
## Multiple R-squared: 0.8716,	Adjusted R-squared: 0.8555 
## F-statistic:  54.3 on 1 and 8 DF,  p-value: 7.852e-05

## y1 <- c(5.24, 7.37, 6.03, 7.03, 7.8, 8.78, 9.28, 8.94, 9.98, 10.18)
## y2 <- c(4.76, 6.25, 5.67, 6.92, 7.25, 8.1, 9.34, 8.99, 10, 10.11)
## x <- seq(0, 9)

## summary(lm(y1~x))

## Call:
## lm(formula = y ~ x)

## Residuals:
##     Min      1Q  Median      3Q     Max 
## -0.7450 -0.3733 -0.1034  0.3616  1.1102 

## Coefficients:
##             Estimate Std. Error t value Pr(>|t|)    
## (Intercept)  5.74455    0.34402  16.698 1.67e-07 ***
## x            0.51521    0.06444   7.995 4.39e-05 ***
## ---
## Signif. codes:  0 ‘***’ 0.001 ‘**’ 0.01 ‘*’ 0.05 ‘.’ 0.1 ‘ ’ 1 

## Residual standard error: 0.5853 on 8 degrees of freedom
## Multiple R-squared: 0.8888,	Adjusted R-squared: 0.8749 
## F-statistic: 63.92 on 1 and 8 DF,  p-value: 4.386e-05 


## summary(lm(y2~x))

## Call:
## lm(formula = y ~ x)

## Residuals:
##     Min      1Q  Median      3Q     Max 
## -0.5673 -0.2697 -0.0640  0.1395  0.7000 

## Coefficients:
##             Estimate Std. Error t value Pr(>|t|)    
## (Intercept)  5.03600    0.25522   19.73 4.53e-08 ***
## x            0.60067    0.04781   12.56 1.51e-06 ***
## ---
## Signif. codes:  0 ‘***’ 0.001 ‘**’ 0.01 ‘*’ 0.05 ‘.’ 0.1 ‘ ’ 1 

## Residual standard error: 0.4342 on 8 degrees of freedom
## Multiple R-squared: 0.9518,	Adjusted R-squared: 0.9457 
## F-statistic: 157.9 on 1 and 8 DF,  p-value: 1.509e-06


pdf("../../doc/sigchi14/figures/R_List_by_Target.pdf", width=7, height=4)
ggplot(data = data.frame(x = c(0,9), y = c(6.740350,10.85737)), aes = aes(x = x, y = y, color="red")) + geom_line()
dev.off()

## fix the color
#### draw the list mode, time vs. position
pdf("../../doc/sigchi14/figures/R_IR_by_Target.pdf")
df.IR <- df[grepl("IR", df$mode), ]
# mapping from id to name
## name <- "GOWFJSYRAM"
## df.IR$target <- sapply(df.IR$target, function(x) substr(name, x, x))
box <- ggplot(df.IR, aes(factor(target), correct_time, fill = factor(target)))
box <- box + geom_boxplot(lwd=0.5) + ylim(2.5,15) + xlab("target by name")
       box + theme(legend.position = "none", axis.title.y=element_blank())
dev.off()
## I don't know how to do annotation
## geom_text(aes(x=label.x, y=3, label=table(df.List$target))) 


#### draw the list mode, time vs. position
mode.bind.type <- cbind(df, paste(df$mode, df$correct_type))
colnames(mode.bind.type)[14] <- "mode_type"

box1 <- ggplot(mode.bind.type, aes(factor(mode), correct_time, fill = mode.bind.type$mode))
box1 <- box1 + geom_boxplot(lwd=0.5) + ylim(2.5,25) + xlab("") + ylab("time (seconds)") +
##  scale_fill_manual(values = c("#A4C3D9", "#9FB987")) + 
  stat_summary(fun.y = "mean", geom = "text", label="---", size= 8, color= "white") +
  annotate("text", x = 1, y = 25, label = "mean: 6.67", size=3.6) +
  annotate("text", x = 1, y = 24, label = "median: 5.77", size=3.6) +
  annotate("text", x = 2, y = 25, label = "mean: 8.86", size=3.6) +
  annotate("text", x = 2, y = 24, label = "median: 7.96", size=3.6) +
  theme_bw() +
  theme(legend.position = "none", axis.title.y=element_blank())

cdf1 <- ggplot(mode.bind.type, aes(x = correct_time, colour = mode)) +
  stat_ecdf() +
  ylab("cumulative distribution function") +
  xlab("acquisition time") +
  xlim(c(0, 20)) +
  theme_bw() +
  theme(legend.justification=c(1,0), legend.position=c(1,0)) +
  scale_fill_discrete(name="test",
                      labels=c("IR selection", "list selection"))


pdf("IR_vs_list.pdf", width = 7, height = 4)
multiplot(box1, cdf1, cols = 2)
dev.off()

## pdf("R_time_by_Category.pdf", width=7, height=4)
## multiplot(box1, box2, cols=2)
## dev.off()

toPlot <- mode.bind.type[grepl("IR", mode.bind.type$mode_type), ]
for (i in 1:nrow(toPlot)) {
  if (toString(toPlot$mode_type[i]) == "IR single") {
    toPlot$type[i] <- "no refinement"
  }
  else {
    toPlot$type[i] <- "with refinement"
  }
}

box2$type <- factor(box2$type, c("without refinement ","with refinement"))

box2 <- ggplot(toPlot, aes(factor(type), correct_time, fill = factor(toPlot$type)))
box2 <- box2 + geom_boxplot(lwd=0.5) + ylim(2.5,25) + xlab("") +
  scale_fill_manual(values = c("#0072B2", "#56B4E9")) +
  annotate("text", x = 1, y = 25, label = "mean: 9.16", size=3.6) +
  annotate("text", x = 1, y = 24, label = "median: 7.67", size=3.6) +
  annotate("text", x = 2, y = 25, label = "mean: 6.40", size=3.6) +
  annotate("text", x = 2, y = 24, label = "median: 5.63", size=3.6) +
  stat_summary(fun.y = "mean", geom = "text", label="---", size= 8, color= "white") +
  theme_bw() +
  theme(legend.position = "none", axis.title.y=element_blank()) +
  scale_x_discrete(labels=c("no refinement", "with refinement"))

cdf2 <- ggplot(toPlot, aes(x = correct_time, colour = type)) +
  scale_colour_manual(values = c("#0072B2", "#56B4E9")) +
  stat_ecdf() +
  ylab("cumulative distribution function") +
  xlab("acquisition time") +
  xlim(c(0, 20)) +
  theme_bw() +
  theme(legend.justification=c(1,0), legend.position=c(1,0)) +
  scale_fill_discrete(name="test",
                      labels=c("IR selection", "list selection"))

pdf("with_and_without_refinement.pdf", width = 7, height = 4)
multiplot(box2, cdf2, cols = 2)
dev.off()


## IR
## summary(mode.bind.type[grepl("IR", mode.bind.type$mode),]$correct_time)
##   Min. 1st Qu.  Median    Mean 3rd Qu.    Max. 
##  1.001   4.611   5.768   6.672   7.392  22.310

## List
## summary(mode.bind.type[grepl("List", mode.bind.type$mode),]$correct_time)
##    Min. 1st Qu.  Median    Mean 3rd Qu.    Max. 
##   3.578   6.084   7.961   8.858   9.931 103.000

## IR single
## summary(mode.bind.type[grepl("single", mode.bind.type$mode_type),]$correct_time)
##    Min. 1st Qu.  Median    Mean 3rd Qu.    Max. 
##   1.001   4.469   5.630   6.399   7.132  22.300 

## IR mul
## summary(mode.bind.type[grepl("IR mul", mode.bind.type$mode_type),]$correct_time)
##    Min. 1st Qu.  Median    Mean 3rd Qu.    Max. 
##   5.130   6.858   7.672   9.159  10.940  22.310


##    IR mul IR single  List mul 
##       19       168       205 
pdf("../../doc/sigchi14/figures/R_mul_vs_single.pdf")
pie(c(19, 168), c("IR multiple", "IR single"))
dev.off()


##  summary(aov(correct_time~mode_type, data=mode.bind.type))
##              Df Sum Sq Mean Sq F value  Pr(>F)    
## mode_type     2    584  292.05    8.52 0.00024 ***
## Residuals   384  13162   34.28                    
## ---
## Signif. codes:  0 ‘***’ 0.001 ‘**’ 0.01 ‘*’ 0.05 ‘.’ 0.1 ‘ ’ 1


## t.test(df.IR.single$correct_time, df.IR.mul$correct_time)

## 	Welch Two Sample t-test

## data:  df.IR.single$correct_time and df.IR.mul$correct_time 
## t = -2.7827, df = 18.875, p-value = 0.01191
## alternative hypothesis: true difference in means is not equal to 0 
## 95 percent confidence interval:
##  -4.8364419 -0.6830473 
## sample estimates:
## mean of x mean of y 
##  6.399340  9.159085



## t.test(df.List$correct_time, df.IR.mul$correct_time)

## 	Welch Two Sample t-test

## data:  df.List$correct_time and df.IR.mul$correct_time 
## t = -0.2738, df = 28.325, p-value = 0.7862
## alternative hypothesis: true difference in means is not equal to 0 
## 95 percent confidence interval:
##  -2.552316  1.950140 
## sample estimates:
## mean of x mean of y 
##  8.857997  9.159085



## t.test(df.List$correct_time, df.IR.single$correct_time)

## 	Welch Two Sample t-test

## data:  df.List$correct_time and df.IR.single$correct_time 
## t = 4.3058, df = 273.772, p-value = 2.32e-05
## alternative hypothesis: true difference in means is not equal to 0 
## 95 percent confidence interval:
##  1.334534 3.582780 
## sample estimates:
## mean of x mean of y 
##  8.857997  6.399340 



##  t.test(df.IR$correct_time, df.List$correct_time)

## 	Welch Two Sample t-test

## data:  df.IR$correct_time and df.List$correct_time 
## t = -3.8107, df = 278.634, p-value = 0.0001706
## alternative hypothesis: true difference in means is not equal to 0 
## 95 percent confidence interval:
##  -3.314811 -1.056619 
## sample estimates:
## mean of x mean of y 
## 6.672282  8.857997 
