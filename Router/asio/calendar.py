from aiogram import Dispatcher

from handlers import unhandled_router, commands_router, text_router


async def calendar(my_bot):
    dp = Dispatcher()

    dp.include_router(unhandled_router.router)
    dp.include_router(commands_router.router)
    dp.include_router(text_router.router)

    await dp.start_polling(my_bot)
