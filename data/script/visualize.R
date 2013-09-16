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
pdf("../../doc/sigchi14/figures/R_List_by_Target.pdf", width=5, height=3)
df.List <- df[grepl("List", df$mode), ]
# mapping from id to name
name <- "GOWFJSYRAM"
df.List$target <- sapply(df.List$target, function(x) substr(name, x, x))
label.x <- sapply(names(table(df.List$target)), function(x) regexpr(x, name))
box <- ggplot(df.List, aes(factor(target), correct_time, fill = df.List$target))
box <- box + geom_boxplot(lwd=0.5) + ylim(2.5,15) + xlab("order in list") + scale_x_discrete(labels=seq(0, 9)) + ylab("time (seconds)")
# linear fit
sorted.name <- sort(unique(df.List$target))
target1 <- sapply(df.List$target, function(x) which(sorted.name == x) - 1)
fit <- lm(df.List$correct_time ~ target1)
target1 <- sapply(df.List$target, function(x) which(sorted.name == x))
box <- box + geom_line(aes(target1, predict(fit)))
box + theme(legend.position = "none")
abline(fit)
dev.off()  


#### draw the list mode, time vs. position
pdf("../../doc/sigchi14/figures/R_IR_by_Target.pdf")

df.IR <- df[grepl("IR", df$mode), ]
# mapping from id to name
## name <- "GOWFJSYRAM"
## df.IR$target <- sapply(df.IR$target, function(x) substr(name, x, x))

box <- ggplot(df.IR, aes(factor(target), correct_time, fill = df.IR$target))
box <- box + geom_boxplot(lwd=0.5) + ylim(2.5,15) + xlab("target by name")
       box + theme(legend.position = "none", axis.title.y=element_blank())
dev.off()
## I don't know how to do annotation
## geom_text(aes(x=label.x, y=3, label=table(df.List$target))) 



#### draw the list mode, time vs. position
mode.bind.type <- cbind(df, paste(df$mode, df$correct_type))
colnames(mode.bind.type)[14] <- "mode_type"
pdf("../../doc/sigchi14/figures/R_time_by_Category.pdf", width=5, height=4)
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




##    IR mul IR single  List mul 
##       19       168       205 
pdf("../../doc/sigchi14/figures/R_mul_vs_single.pdf")
pie(c(19, 168), c("IR multiple", "IR single"))
dev.off()

## > summary(aov(correct_time~mode_type, data=mode.bind.type))
##              Df Sum Sq Mean Sq F value Pr(>F)  
## mode_type     2    316  157.90   2.977 0.0521 .
## Residuals   389  20632   53.04                 
## ---
## Signif. codes:  0 ‘***’ 0.001 ‘**’ 0.01 ‘*’ 0.05 ‘.’ 0.1 ‘ ’ 1 
