import asyncio
import logging

from aiogram import Bot

from asio.calendar import calendar
from asio.notifier import notifier

from misc.config_reader import config


async def main():
    my_bot = Bot(token=config.bot_token.get_secret_value(), parse_mode="MarkdownV2")
    await asyncio.gather(calendar(my_bot), notifier(my_bot))


if __name__ == '__main__':
    logging.basicConfig(filename="log.txt", level=logging.INFO)
    asyncio.run(main())
