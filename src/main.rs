use ratatui::{widgets::Paragraph, Frame};
use crossterm::event::{self, Event, KeyCode, KeyEvent, KeyEventKind};

fn handle_events() -> std::io::Result<bool> {
    match event::read()? {
        Event::Key(key) if key.kind == KeyEventKind::Press => match key.code {
            KeyCode::Char('q') => return Ok(true),
            // handle other key events
            _ => {}
        },
        // handle other events
        _ => {}
    }
    Ok(false)
}

fn run(terminal: &mut ratatui::DefaultTerminal) -> std::io::Result<()> {
    loop {
        terminal.draw(|frame| draw(frame))?;
        if handle_events()? {
            break Ok(());
        }
    }
}

fn draw(frame: &mut Frame) {
    let text = Paragraph::new("Hello World!");
    frame.render_widget(text, frame.area());
}

fn main() -> std::io::Result<()> {
    let mut terminal = ratatui::init();
    let result = run(&mut terminal);
    ratatui::restore();
    result
}
