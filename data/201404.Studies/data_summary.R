d <- read.table("aggregate.txt")

summary(d[d$V5=="intensity", 3:4])
summary(d[d$V5=="name", 3:4])
