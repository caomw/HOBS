require('ggplot2')

## read data in first
df <- read.csv("data.csv")

## discard data frmo valkerie and Micheal
df <- df[!is.na(df$correct_time),]
df <- df[!(grepl("valkerie", df$participant) | grepl("Micheal", df$participant)), ]

df.IR.single <- df[grepl("IR", df$mode) & grepl("single", df$correct_type), ]
df.IR.mul <- df[grepl("IR", df$mode) & grepl("mul", df$correct_type), ]


#### draw the list mode, time vs. position
pdf("List_by_Target.pdf")

df.List <- df[grepl("List", df$mode), ]
# mapping from id to name
name <- "GOWFJSYRAM"
df.List$target <- sapply(df.List$target, function(x) substr(name, x, x))

label.x <- sapply(names(table(df.List$target)), function(x) regexpr(x, name))
box <- ggplot(df.List, aes(factor(target), correct_time, fill = df.List$target))
box <- box + geom_boxplot(lwd=0.5) + ylim(2.5,15) + xlab("target by name")
box + theme(legend.position = "none", axis.title.y=element_blank()) +

## I don't know how to do annotation
## geom_text(aes(x=label.x, y=3, label=table(df.List$target))) 
dev.off()



#### draw the list mode, time vs. position
mode.bind.type <- cbind(df, paste(df$mode, df$correct_type))
colnames(mode.bind.type)[14] <- "mode_type"

pdf("time_by_Category.pdf")
toPlot <- mode.bind.type
box <- ggplot(toPlot, aes(factor(mode_type), correct_time, fill = toPlot$mode_type))
box <- box + geom_boxplot(lwd=0.5) + ylim(2.5,25) + xlab("different mode")
       box + theme(legend.position = "none", axis.title.y=element_blank())
dev.off()

##    IR mul IR single  List mul 
##       19       168       205 
pdf("mul_vs_single.pdf")
pie(c(19, 168), c("IR multiple", "IR single"))
dev.off()

