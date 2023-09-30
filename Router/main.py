import asyncio
import logging

from aiogram import Bot, Dispatcher

from misc.config_reader import config

from handlers import text_router


async def main():
    bot = Bot(token=config.bot_token.get_secret_value(), parse_mode="MarkdownV2")
    dp = Dispatcher()

    dp.include_router(text_router.router)

    await dp.start_polling(bot)


if __name__ == '__main__':
    logging.basicConfig(filename="log.txt", level=logging.INFO)
    asyncio.run(main())
