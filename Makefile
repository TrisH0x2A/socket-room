CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -g \
           -Iinclude \
           -Ivendor \
           -DMG_ENABLE_OPENSSL=0 \
           -DMG_ENABLE_LOG=1 \
           -Wno-format-truncation \
           -Wno-implicit-fallthrough \
           -Wno-discarded-qualifiers \
           -Wno-unused-parameter

LDFLAGS = -lpthread -lm -ldl

SRC = src/main.c \
      src/server.c \
      src/router.c \
      src/websocket.c \
      src/auth.c \
      src/users.c \
      src/rooms.c \
      src/messages.c \
      src/db.c \
      src/utils.c \
      vendor/mongoose.c \
      vendor/sqlite3.c

TARGET = build/socketroom

.PHONY: all run clean frontend release

all: $(TARGET)

$(TARGET): $(SRC)
	@mkdir -p build data static
	$(CC) $(CFLAGS) -o $@ $(SRC) $(LDFLAGS)
	@echo "\n✓ Built: $(TARGET)"

run: all
	./$(TARGET)

run-server: run

run-web:
	cd frontend && pnpm run dev

install-web:
	cd frontend && pnpm install

frontend:
	@echo "→ Installing frontend dependencies..."
	cd frontend && pnpm install
	@echo "→ Building frontend..."
	cd frontend && pnpm run build
	@echo "→ Copying dist to static/..."
	@mkdir -p static
	cp -r frontend/dist/. static/
	@echo "✓ Frontend built and copied to static/"

release: frontend all
	@echo "\n✓ Release build complete. Run with: ./$(TARGET)"

clean:
	rm -rf build/ static/
	cd frontend && rm -rf dist/ node_modules/ 2>/dev/null || true
	@echo "✓ Cleaned"
