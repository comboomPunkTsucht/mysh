use std::error::Error;
use std::io::stdout;
use std::time::Duration;

use color_eyre::Result;
use ratatui::{prelude::*, widgets::*};
use ratatui_splash_screen::{SplashConfig, SplashScreen, SplashError};
use tokio::sync::mpsc::UnboundedSender;

use super::Component;
use crate::{action::Action, config::Config};

#[derive(Default)]
pub struct Splash {
    command_tx: Option<UnboundedSender<Action>>,
    config: Config,
}

impl Splash {
    pub fn new() -> Self {
        Self::default()
    }
}

static SPLASH_CONFIG: SplashConfig = SplashConfig {
    image_data: include_bytes!("../../assets/logo.png"),
    sha256sum: Some("24b788f3acd908e5254ae8d72c407a0274643777ed9a45d0c6d51346cde59436"),
    render_steps: 12,
    use_colors: true,
};

impl Component for Splash {
    fn register_action_handler(&mut self, tx: UnboundedSender<Action>) -> Result<()> {
        self.command_tx = Some(tx);
        Ok(())
    }

    fn register_config_handler(&mut self, config: Config) -> Result<()> {
        self.config = config;
        Ok(())
    }

    fn update(&mut self, action: Action) -> Result<Option<Action>> {
        match action {
            Action::Tick => {
                // add any logic here that should run on every tick
            }
            Action::Render => {
                // add any logic here that should run on every render
            }
            _ => {}
        }
        Ok(None)
    }

    fn draw(&mut self, frame: &mut Frame, area: Rect) -> Result<()> {
        let mut splash_screen = SplashScreen::new(SPLASH_CONFIG)?;
        while !splash_screen.is_rendered() {
            frame.render_widget(&mut splash_screen, frame.size());
        std::thread::sleep(Duration::from_millis(100));
    }
        Ok(())
    }
}
