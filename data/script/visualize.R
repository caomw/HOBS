require('ggplot2')

# Multiple plot function
#
# ggplot objects can be passed in ..., or to plotlist (as a list of ggplot objects)
# - cols:   Number of columns in layout
# - layout: A matrix specifying the layout. If present, 'cols' is ignored.
#
# If the layout is something like matrix(c(1,2,3,3), nrow=2, byrow=TRUE),
# then plot 1 will go in the upper left, 2 will go in the upper right, and
# 3 will go all the way across the bottom.
#
multiplot <- function(..., plotlist=NULL, file, cols=1, layout=NULL) {
  require(grid)

  # Make a list from the ... arguments and plotlist
  plots <- c(list(...), plotlist)

  numPlots = length(plots)

  # If layout is NULL, then use 'cols' to determine layout
  if (is.null(layout)) {
    # Make the panel
    # ncol: Number of columns of plots
    # nrow: Number of rows needed, calculated from # of cols
    layout <- matrix(seq(1, cols * ceiling(numPlots/cols)),
                    ncol = cols, nrow = ceiling(numPlots/cols))
  }

 if (numPlots==1) {
    print(plots[[1]])

  } else {
    # Set up the page
    grid.newpage()
    pushViewport(viewport(layout = grid.layout(nrow(layout), ncol(layout))))

    # Make each plot, in the correct location
    for (i in 1:numPlots) {
      # Get the i,j matrix positions of the regions that contain this subplot
      matchidx <- as.data.frame(which(layout == i, arr.ind = TRUE))

      print(plots[[i]], vp = viewport(layout.pos.row = matchidx$row,
                                      layout.pos.col = matchidx$col))
    }
  }
}


## read data in first
df <- read.csv("data.csv")

## discard data frmo valkerie and Micheal
df <- df[!is.na(df$correct_time),]
df <- df[!(grepl("valkerie", df$participant) | grepl("Micheal", df$participant)), ]

df.IR.single <- df[grepl("IR", df$mode) & grepl("single", df$correct_type), ]
df.IR.mul <- df[grepl("IR", df$mode) & grepl("mul", df$correct_type), ]


#### draw the list mode, time vs. position
pdf("../../doc/sigchi14/figures/R_List_by_Target.pdf", width=7, height=4)
df.List <- df[grepl("List", df$mode), ]
# mapping from id to name
name <- "GOWFJSYRAM"
df.List$target <- sapply(df.List$target, function(x) substr(name, x, x))
label.x <- sapply(names(table(df.List$target)), function(x) regexpr(x, name))
box <- ggplot(df.List, aes(factor(target), correct_time, fill = df.List$target))
box <- box + geom_boxplot(lwd=0.5) + ylim(2.5,15) + xlab("order in list") + scale_x_discrete(labels=seq(0, 9)) + ylab("time (seconds)")
## linear fit
sorted.name <- sort(unique(df.List$target))
target1 <- sapply(df.List$target, function(x) which(sorted.name == x) - 1)
fit <- lm(df.List$correct_time ~ target1)
# geom_line(x = c(0,9), y = c(6.740350,10.85737), colour = "red")
box + theme(legend.position = "none") 
dev.off()

## > summary(fit)

## Call:
## lm(formula = df.List$correct_time ~ target1)

## Residuals:
##    Min     1Q Median     3Q    Max 
## -3.904 -2.106 -1.066  0.260 95.364 

## Coefficients:
##             Estimate Std. Error t value Pr(>|t|)    
## (Intercept)   6.7403     0.9685   6.959 4.62e-11 ***
## target1       0.4574     0.1768   2.588   0.0104 *  
## ---
## Signif. codes:  0 ‘***’ 0.001 ‘**’ 0.01 ‘*’ 0.05 ‘.’ 0.1 ‘ ’ 1 

## Residual standard error: 7.416 on 203 degrees of freedom
## Multiple R-squared: 0.03193,	Adjusted R-squared: 0.02716 
## F-statistic: 6.696 on 1 and 203 DF,  p-value: 0.01036 


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
pdf("../../doc/sigchi14/figures/R_time_by_Category.pdf", width=7, height=4)
box1 <- ggplot(mode.bind.type, aes(factor(mode), correct_time, fill = mode.bind.type$mode))
box1 <- box1 + geom_boxplot(lwd=0.5) + ylim(2.5,25) + xlab("different mode") +
    theme(legend.position = "none") +
    stat_summary(fun.y = "mean", geom = "text", label="---", size= 8, color= "white") +
    ylab("time (seconds)")
toPlot <- mode.bind.type[grepl("IR", mode.bind.type$mode_type), ]
box2 <- ggplot(toPlot, aes(factor(mode_type), correct_time, fill = toPlot$mode_type))
box2 <- box2 + geom_boxplot(lwd=0.5) + ylim(2.5,25) + xlab("IR mode") +
    theme(legend.position = "none", axis.title.y=element_blank()) +
    scale_fill_manual(values = c("pink", "green")) +
    stat_summary(fun.y = "mean", geom = "text", label="---", size= 8, color= "white")
multiplot(box1, box2, cols=2)
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
