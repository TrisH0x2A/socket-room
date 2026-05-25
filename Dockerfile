FROM alpine:latest AS builder

RUN apk add --no-cache gcc musl-dev make
WORKDIR /app
COPY . .
RUN make

FROM alpine:latest
WORKDIR /app

COPY --from=builder /app/build/socketroom ./build/socketroom
COPY --from=builder /app/static ./static

RUN mkdir -p data

EXPOSE 8080
CMD ["./build/socketroom"]
